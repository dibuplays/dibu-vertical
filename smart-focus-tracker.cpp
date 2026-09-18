#include "smart-focus-tracker.hpp"

#include <algorithm>
#include <cmath>

std::vector<uint8_t> SmartFocusTracker::ToGray(const QImage &frame)
{
	QImage image = frame.convertToFormat(QImage::Format_RGBA8888);
	std::vector<uint8_t> gray(size_t(image.width()) * size_t(image.height()));
	for (int y = 0; y < image.height(); ++y) {
		const auto *line = image.constScanLine(y);
		for (int x = 0; x < image.width(); ++x) {
			const int offset = x * 4;
			gray[size_t(y) * size_t(image.width()) + size_t(x)] =
				uint8_t((77 * line[offset] + 150 * line[offset + 1] + 29 * line[offset + 2]) >> 8);
		}
	}
	return gray;
}

bool SmartFocusTracker::SetTarget(const QImage &frame, const QRect &requestedBounds)
{
	Clear();
	if (frame.isNull())
		return false;

	frameWidth = frame.width();
	frameHeight = frame.height();
	bounds = requestedBounds.normalized().intersected(frame.rect());
	if (bounds.width() < 8 || bounds.height() < 8)
		return false;

	// A compact template keeps tracking responsive inside OBS. Large selections
	// are sampled rather than allocating an expensive full-size search template.
	if (bounds.width() > 64)
		bounds.setWidth(64);
	if (bounds.height() > 80)
		bounds.setHeight(80);
	bounds.moveCenter(requestedBounds.normalized().center());
	bounds = bounds.intersected(frame.rect());

	auto gray = ToGray(frame);
	target.resize(size_t(bounds.width()) * size_t(bounds.height()));
	for (int y = 0; y < bounds.height(); ++y) {
		std::copy_n(gray.data() + size_t(bounds.y() + y) * size_t(frameWidth) + size_t(bounds.x()), bounds.width(),
			    target.data() + size_t(y) * size_t(bounds.width()));
	}
	originalTarget = target;
	return true;
}

float SmartFocusTracker::CorrelationAt(const std::vector<uint8_t> &gray, const std::vector<uint8_t> &reference,
				       int width, int x, int y, int sampleStep) const
{
	double sumReference = 0.0;
	double sumCandidate = 0.0;
	double sumReferenceSquared = 0.0;
	double sumCandidateSquared = 0.0;
	double sumProduct = 0.0;
	int samples = 0;
	for (int ty = 0; ty < bounds.height(); ty += sampleStep) {
		const auto *candidate = gray.data() + size_t(y + ty) * size_t(width) + size_t(x);
		const auto *referenceLine = reference.data() + size_t(ty) * size_t(bounds.width());
		for (int tx = 0; tx < bounds.width(); tx += sampleStep) {
			const double r = referenceLine[tx];
			const double c = candidate[tx];
			sumReference += r;
			sumCandidate += c;
			sumReferenceSquared += r * r;
			sumCandidateSquared += c * c;
			sumProduct += r * c;
			++samples;
		}
	}
	if (samples < 16)
		return -1.0f;
	const double numerator = samples * sumProduct - sumReference * sumCandidate;
	const double referenceEnergy = samples * sumReferenceSquared - sumReference * sumReference;
	const double candidateEnergy = samples * sumCandidateSquared - sumCandidate * sumCandidate;
	const double denominator = std::sqrt(std::max(0.0, referenceEnergy * candidateEnergy));
	return denominator > 0.0001 ? float(numerator / denominator) : -1.0f;
}

SmartFocusTracker::Match SmartFocusTracker::FindBest(const std::vector<uint8_t> &gray,
						      const std::vector<uint8_t> &reference, const QRect &requestedArea,
						      int searchStep, int sampleStep) const
{
	Match best;
	const int maxFrameX = frameWidth - bounds.width();
	const int maxFrameY = frameHeight - bounds.height();
	const int minX = std::clamp(requestedArea.left(), 0, maxFrameX);
	const int maxX = std::clamp(requestedArea.right(), 0, maxFrameX);
	const int minY = std::clamp(requestedArea.top(), 0, maxFrameY);
	const int maxY = std::clamp(requestedArea.bottom(), 0, maxFrameY);
	for (int y = minY; y <= maxY; y += searchStep) {
		for (int x = minX; x <= maxX; x += searchStep) {
			const float score = CorrelationAt(gray, reference, frameWidth, x, y, sampleStep);
			if (score > best.score) {
				best.score = score;
				best.position = QPoint(x, y);
			}
		}
	}
	return best;
}

void SmartFocusTracker::RefreshTemplate(const std::vector<uint8_t> &gray, int width, const QRect &newBounds)
{
	for (int y = 0; y < newBounds.height(); ++y) {
		const auto *candidate = gray.data() + size_t(newBounds.y() + y) * size_t(width) + size_t(newBounds.x());
		auto *reference = target.data() + size_t(y) * size_t(newBounds.width());
		for (int x = 0; x < newBounds.width(); ++x)
			reference[x] = uint8_t((int(reference[x]) * 63 + int(candidate[x])) / 64);
	}
}

SmartFocusTracker::Result SmartFocusTracker::Track(const QImage &frame)
{
	Result result;
	if (!IsReady() || frame.isNull() || frame.width() != frameWidth || frame.height() != frameHeight)
		return result;

	auto gray = ToGray(frame);
	const int radiusX = std::max(24, frameWidth / 9);
	const int radiusY = std::max(24, frameHeight / 11);
	const QPoint predicted = bounds.topLeft() + QPoint(qRound(velocity.x()), qRound(velocity.y()));
	const QRect localArea(predicted.x() - radiusX, predicted.y() - radiusY, radiusX * 2 + 1, radiusY * 2 + 1);
	Match best = FindBest(gray, target, localArea, 2, 2);
	if (best.score >= 0.58f) {
		const QRect refineArea(best.position.x() - 3, best.position.y() - 3, 7, 7);
		best = FindBest(gray, target, refineArea, 1, 1);
	}

	result.confidence = std::max(0.0f, best.score);
	result.bounds = QRect(best.position, bounds.size());
	result.found = best.score >= 0.64f;
	if (!result.found) {
		++lostFrames;
		velocity *= 0.55;
		// Every sixth miss, search the whole canvas using the untouched original
		// target. This recovers from temporary occlusion without teaching the
		// tracker that a piece of background is the character.
		if (lostFrames % 6 == 0) {
			const QRect wholeFrame(0, 0, frameWidth - bounds.width() + 1, frameHeight - bounds.height() + 1);
			best = FindBest(gray, originalTarget, wholeFrame, 4, 2);
			if (best.score >= 0.72f) {
				const QRect refineArea(best.position.x() - 4, best.position.y() - 4, 9, 9);
				best = FindBest(gray, originalTarget, refineArea, 1, 1);
				result.confidence = std::max(0.0f, best.score);
				result.bounds = QRect(best.position, bounds.size());
				result.found = best.score >= 0.72f;
				result.reacquired = result.found;
			}
		}
	}
	if (result.found) {
		const QPoint movement = result.bounds.topLeft() - bounds.topLeft();
		velocity = velocity * 0.65 + QPointF(movement) * 0.35;
		bounds = result.bounds;
		lostFrames = 0;
		if (result.confidence >= 0.86f && !result.reacquired)
			RefreshTemplate(gray, frameWidth, bounds);
	}
	return result;
}

void SmartFocusTracker::Clear()
{
	target.clear();
	originalTarget.clear();
	bounds = {};
	velocity = {};
	lostFrames = 0;
	frameWidth = 0;
	frameHeight = 0;
}

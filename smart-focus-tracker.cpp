#include "smart-focus-tracker.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

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
	if (bounds.width() > 72)
		bounds.setWidth(72);
	if (bounds.height() > 72)
		bounds.setHeight(72);
	bounds.moveCenter(requestedBounds.normalized().center());
	bounds = bounds.intersected(frame.rect());

	auto gray = ToGray(frame);
	target.resize(size_t(bounds.width()) * size_t(bounds.height()));
	for (int y = 0; y < bounds.height(); ++y) {
		std::copy_n(gray.data() + size_t(bounds.y() + y) * size_t(frameWidth) + size_t(bounds.x()), bounds.width(),
			    target.data() + size_t(y) * size_t(bounds.width()));
	}
	return true;
}

uint64_t SmartFocusTracker::DifferenceAt(const std::vector<uint8_t> &gray, int width, int x, int y) const
{
	uint64_t difference = 0;
	// Two-pixel sampling substantially reduces work while retaining enough
	// detail for a character-sized selection.
	for (int ty = 0; ty < bounds.height(); ty += 2) {
		const auto *candidate = gray.data() + size_t(y + ty) * size_t(width) + size_t(x);
		const auto *reference = target.data() + size_t(ty) * size_t(bounds.width());
		for (int tx = 0; tx < bounds.width(); tx += 2)
			difference += uint64_t(std::abs(int(candidate[tx]) - int(reference[tx])));
	}
	return difference;
}

void SmartFocusTracker::RefreshTemplate(const std::vector<uint8_t> &gray, int width, const QRect &newBounds)
{
	for (int y = 0; y < newBounds.height(); ++y) {
		const auto *candidate = gray.data() + size_t(newBounds.y() + y) * size_t(width) + size_t(newBounds.x());
		auto *reference = target.data() + size_t(y) * size_t(newBounds.width());
		for (int x = 0; x < newBounds.width(); ++x)
			reference[x] = uint8_t((int(reference[x]) * 15 + int(candidate[x])) / 16);
	}
}

SmartFocusTracker::Result SmartFocusTracker::Track(const QImage &frame)
{
	Result result;
	if (!IsReady() || frame.isNull() || frame.width() != frameWidth || frame.height() != frameHeight)
		return result;

	auto gray = ToGray(frame);
	const int radiusX = std::max(18, frameWidth / 10);
	const int radiusY = std::max(18, frameHeight / 12);
	const int minX = std::max(0, bounds.x() - radiusX);
	const int maxX = std::min(frameWidth - bounds.width(), bounds.x() + radiusX);
	const int minY = std::max(0, bounds.y() - radiusY);
	const int maxY = std::min(frameHeight - bounds.height(), bounds.y() + radiusY);

	uint64_t bestDifference = std::numeric_limits<uint64_t>::max();
	QPoint best = bounds.topLeft();
	for (int y = minY; y <= maxY; y += 2) {
		for (int x = minX; x <= maxX; x += 2) {
			const uint64_t difference = DifferenceAt(gray, frameWidth, x, y);
			if (difference < bestDifference) {
				bestDifference = difference;
				best = QPoint(x, y);
			}
		}
	}

	const uint64_t samples = uint64_t((bounds.width() + 1) / 2) * uint64_t((bounds.height() + 1) / 2);
	result.confidence = samples ? 1.0f - float(bestDifference) / float(samples * 255ULL) : 0.0f;
	result.bounds = QRect(best, bounds.size());
	result.found = result.confidence >= 0.56f;
	if (result.found) {
		bounds = result.bounds;
		if (result.confidence >= 0.72f)
			RefreshTemplate(gray, frameWidth, bounds);
	}
	return result;
}

void SmartFocusTracker::Clear()
{
	target.clear();
	bounds = {};
	frameWidth = 0;
	frameHeight = 0;
}

#pragma once

#include <QImage>
#include <QRect>
#include <vector>

class SmartFocusTracker {
public:
	struct Result {
		QRect bounds;
		float confidence = 0.0f;
		bool found = false;
	};

	bool SetTarget(const QImage &frame, const QRect &bounds);
	Result Track(const QImage &frame);
	void Clear();
	bool IsReady() const { return !target.empty(); }
	QRect Bounds() const { return bounds; }

private:
	static std::vector<uint8_t> ToGray(const QImage &frame);
	uint64_t DifferenceAt(const std::vector<uint8_t> &gray, int frameWidth, int x, int y) const;
	void RefreshTemplate(const std::vector<uint8_t> &gray, int frameWidth, const QRect &newBounds);

	std::vector<uint8_t> target;
	QRect bounds;
	int frameWidth = 0;
	int frameHeight = 0;
};

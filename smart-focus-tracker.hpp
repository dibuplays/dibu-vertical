#pragma once

#include <QImage>
#include <QPointF>
#include <QRect>
#include <vector>

class SmartFocusTracker {
public:
	struct Result {
		QRect bounds;
		float confidence = 0.0f;
		bool found = false;
		bool reacquired = false;
	};

	bool SetTarget(const QImage &frame, const QRect &bounds);
	Result Track(const QImage &frame);
	void Clear();
	bool IsReady() const { return !target.empty(); }
	QRect Bounds() const { return bounds; }

private:
	struct Match {
		QPoint position;
		float score = -1.0f;
	};

	static std::vector<uint8_t> ToGray(const QImage &frame);
	float CorrelationAt(const std::vector<uint8_t> &gray, const std::vector<uint8_t> &reference, int frameWidth,
			    int x, int y, int sampleStep) const;
	Match FindBest(const std::vector<uint8_t> &gray, const std::vector<uint8_t> &reference, const QRect &area,
		       int searchStep, int sampleStep) const;
	void RefreshTemplate(const std::vector<uint8_t> &gray, int frameWidth, const QRect &newBounds);

	std::vector<uint8_t> target;
	std::vector<uint8_t> originalTarget;
	QRect bounds;
	QPointF velocity;
	int lostFrames = 0;
	int frameWidth = 0;
	int frameHeight = 0;
};

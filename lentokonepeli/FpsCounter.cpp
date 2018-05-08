#pragma once

#include "FpsCounter.h"
#include "Master.h"

void FpsCounter::updateFrameTimes(float newItem) {
	if (myMaster->settings.showFps == true) {
		frameTimes.push_front(newItem);
		while (frameTimes.size() > 100) {
			frameTimes.pop_back();
		}
	}
}

int FpsCounter::getCurrentFps() {
	float average = 0.0F;
	for (float frameTime : frameTimes) {
		average = (average + frameTime) / 2.0F;
	}
	return (int)roundf(1.0F / average);
}
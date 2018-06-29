#pragma once

#include "CountdownTimer.h"

class FlickerCountdownTimer : public CountdownTimer {
public: 
	FlickerCountdownTimer(float _startTime, float _flickerInterval) : CountdownTimer(_startTime), flickerInterval{ _flickerInterval } {}

	bool flickerOn() { return isDone() == false && static_cast<int>(ceilf(clock.getElapsedTime().asSeconds() / flickerInterval)) % 2 == 1; }
	bool flickerOff() { return !flickerOn(); }

private:
	float flickerInterval = 0.0F;
};
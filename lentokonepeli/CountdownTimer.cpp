#include "CountdownTimer.h"

void CountdownTimer::start(float newStartTime) {
	if (newStartTime > 0.0F) {
		startTime = newStartTime;
	}
	started = true;
	clock.restart();
}

bool CountdownTimer::isDone() {
	return getTimeRemaining() <= 0.0F;
}

float CountdownTimer::getTimeRemaining() {
	if (!started)
		return -1.0F;
	return startTime - clock.getElapsedTime().asSeconds();
}

float CountdownTimer::getTimeRemainingRatio() {
	if(!started)
		return 0.0F;
	return (startTime - clock.getElapsedTime().asSeconds()) / startTime;
}
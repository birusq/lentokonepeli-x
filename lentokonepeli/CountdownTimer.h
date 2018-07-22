#pragma once

#include <SFML/System.hpp>


class CountdownTimer {
public:
	CountdownTimer() {};
	CountdownTimer(float _startTime) : startTime{ _startTime } {};

	void start(float newStartTime = -1.0F);

	bool isDone();

	float getTimeRemaining();

	float getTimeRemainingRatio();

protected: 
	bool started = false;
	float startTime = 0.0F;
	sf::Clock clock;
};
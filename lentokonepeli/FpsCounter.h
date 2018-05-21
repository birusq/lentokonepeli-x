#pragma once

#include <deque>
class Master;

class FpsCounter {
public:

	std::deque<float> frameTimes;

	void updateFrameTimes(float newItem);

	int getCurrentFps();
};
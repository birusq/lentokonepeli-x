#pragma once

#include <deque>
class Master;

class FpsCounter {
public:
	FpsCounter(Master* master_) : myMaster{ master_ } {}

	std::deque<float> frameTimes;

	void updateFrameTimes(float newItem);

	int getCurrentFps();
private:
	Master* myMaster;
};
#pragma once

#include "RakNet/WindowsIncludes.h"
#include <SFML\System.hpp>
#include <unordered_map>
#include "GOManager.h"
#include "FpsCounter.h"
#include "Closeable.h"
#include "Master.h"
#include "TestLevel.h"

class Master;
class GUI;

struct Input {
	bool moveForward = false;
	bool turnLeft = false;
	bool turnRight = false;
	bool shooting = false;
};

class Game : public FpsCounter, public Closeable {
public:
	virtual void loop() = 0;
protected:
	Game(Master* master_);

	Master* master;

	const float fixedUpdateFps = 60.0F;
	const sf::Vector2f gravity{0, 10.0F};

	void integrate(PhysicsTransformable& pTrans, float dt);

	// Call when player presses throttle
	void improveHandling(Ship& ship);

	GUI* gui;

	GOManager goManager;
	
	void quit();

	TestLevel level;
};


#pragma once

#include "RakNet/WindowsIncludes.h"
#include <SFML\System.hpp>
#include <unordered_map>
#include "GOManager.h"
#include "FpsCounter.h"
#include "Closeable.h"
#include "TestLevel.h"

struct Input {
	bool moveForward = false;
	bool turnLeft = false;
	bool turnRight = false;
	bool shooting = false;
	bool any() { return (moveForward || turnLeft || turnRight || shooting); }
};

class Game : public FpsCounter, public Closeable {
public:
	virtual void loop() = 0;
protected:

	const float fixedUpdateFps = 60.0F;
	const sf::Vector2f gravity{0, 10.0F};

	void integrate(PhysicsTransformable& pTrans, float dt);

	// Call when player presses throttle
	void improveHandling(Ship& ship);

	void collisionDetectAll(std::unordered_map<Team::Id, Team>& teams);

	virtual void onBulletCollision(Bullet& bullet, Ship& targetShip) = 0;
	virtual void onShipCollision(Ship& s1, Ship& s2) = 0;

	GOManager goManager;
	
	void quit();

	TestLevel level;
};


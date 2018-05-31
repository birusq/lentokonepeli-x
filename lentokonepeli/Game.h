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
	virtual void spawnShip(sf::Uint8 clientId) = 0;

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

	std::unordered_map<sf::Uint8, float> spawnTimers;
	
	// checks timers and spawns ships if timer allows
	void handleSpawnTimers(float dt);
};


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
	bool abilityForward = false;
	bool abilityLeft = false;
	bool abilityBackward = false;
	bool abilityRight = false;
	bool precisionTurn = false;

	bool any() { return (moveForward || turnLeft || turnRight || shooting || abilityForward || abilityLeft || abilityBackward || abilityRight || precisionTurn); }
};

struct InputResponse {
	int bulletId = -1;
	bool isUsingAbility = false;
};

class Game : public FpsCounter, public Closeable {
public:
	Game();

	virtual void loop() = 0;

	virtual void onShipDeath(Ship* ship) = 0;

	GOManager goManager;

	TestLevel level;

	Scores scores;

	bool isServer() { return isserver; }

protected:
	virtual void spawnShip(sf::Uint8 clientId) = 0;

	const float fixedUpdateFps = 60.0F;
	const sf::Vector2f gravity{0, 10.0F};

	void integrate(PhysicsTransformable& pTrans, float dt);

	// Call when player presses throttle
	void improveHandling(Ship& ship);

	void collisionDetectAll(std::unordered_map<Team::Id, Team>& teams);

	void updateAllHitboxPositions();

	virtual void onBulletCollision(Bullet& bullet, Ship& targetShip) = 0;
	virtual void onShipToShipCollision(Ship& s1, Ship& s2) = 0;
	virtual void onShipToGroundCollision(Ship& ship) = 0;

	void quit();

	virtual void onQuit() = 0;

	std::unordered_map<sf::Uint8, CountdownTimer> spawnTimers;
	
	// checks timers and spawns ships if timer allows
	virtual void handleSpawnTimers(float dt);

	// Moves ship back to spawn point and removes velocity and acceleration
	void resetShipTransform(Ship& ship);

	bool isserver = false;
};


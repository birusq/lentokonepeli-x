#pragma once

#include <SFML\Graphics.hpp>
#include <unordered_map>
#include "PhysicsTransformable.h"
#include "Ship.h"
#include "Packethelper.h"
#include "User.h"
#include "Bullet.h"

class GOManager {
public:
	std::unordered_map<sf::Uint32, PhysicsTransformable> previousPTransformsState;
	std::unordered_map<sf::Uint32, PhysicsTransformable> currentPTransformsState;
	std::unordered_map<sf::Uint32, PhysicsTransformable*> pTransPointers;

	std::unordered_map<sf::Uint8, Ship> ships;
	std::unordered_map<sf::Uint8, std::unordered_map<sf::Uint16, Bullet>> bullets;
	

	sf::Uint32 getUnusedPTransId();
	void addToPhysics(PhysicsTransformable* pTrans);
	void removeFromPhysics(PhysicsTransformable* pTrans);

	void applyTransforms(std::unordered_map<sf::Uint32, PhysicsTransformable>& state);

	void drawAll(sf::RenderWindow& window);

	void createShip(User* const user, TeamId teamId = NO_TEAM);
	void removeShip(sf::Uint8 clientId);

	sf::Uint16 getNewBulletId(sf::Uint8 forClientId);
	Bullet* createBullet(sf::Uint8 ClientId, sf::Uint16 bulletId);
	void removeBullet(sf::Uint8 clientId, sf::Uint16 bulletId);

	ShipState getShipState(sf::Uint8 clientIndex);
};
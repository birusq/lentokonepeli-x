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

	void applyTransforms(std::unordered_map<sf::Uint32, PhysicsTransformable>& state);

	void drawAll(sf::RenderWindow& window);

	void createShip(User* user, TeamId teamId = NO_TEAM);
	void removeShip(sf::Uint8 clientId);

	sf::Uint16 getNewBulletId(sf::Uint8 forClientId);
	Bullet* createBullet(sf::Uint8 clientId, sf::Uint16 bulletId);
	void removeBullet(sf::Uint8 clientId, sf::Uint16 bulletId);

	ShipState getShipState(sf::Uint8 clientIndex);

	void addToPhysics(PhysicsTransformable* pTrans);

	// Delete unused objects here to not invalidate iterators
	void deleteGarbage();
private:
	std::vector<std::pair<sf::Uint8, sf::Uint16>> bulletGarbage;
	std::vector<sf::Uint8> shipGarbage;
	
	void removeFromPhysics(PhysicsTransformable* pTrans);
};
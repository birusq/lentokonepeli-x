#pragma once

#include <SFML\Graphics.hpp>
#include <unordered_map>
#include "PhysicsTransformable.h"
#include "Ship.h"
#include "Packethelper.h"
#include "User.h"
#include "Bullet.h"

enum ObjectType : sf::Uint8 { 
	SHIP,
	BULLET
};

class GOManager {
public:
	std::unordered_map<ObjectType, std::unordered_map<sf::Uint32, PhysicsTransformable>> previousPTransforms{};
	std::unordered_map<ObjectType, std::unordered_map<sf::Uint32, PhysicsTransformable>> currentPTransforms{};

	std::unordered_map<sf::Uint8, Ship> ships;
	std::unordered_map<sf::Uint8, std::unordered_map<sf::Uint8, Bullet*>> bullets;


	void applyTransforms(std::unordered_map<ObjectType, std::unordered_map<sf::Uint32, PhysicsTransformable>>& state);

	void drawAll(sf::RenderWindow& window);

	void createShip(User* const user, TeamId teamId = NO_TEAM);
	void removeShip(sf::Uint8 clientId);

	ShipState getShipState(sf::Uint8 clientIndex);
};
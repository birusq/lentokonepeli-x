#pragma once

#include <SFML\Graphics.hpp>
#include <unordered_map>
#include "PhysicsTransformable.h"
#include "Ship.h"
#include "Packethelper.h"
#include "User.h"

enum ObjectType : uchar { 
	SHIP,
	BULLET
};

class GOManager {
public:
	std::unordered_map<ObjectType, std::unordered_map<uint, PhysicsTransformable>> previousPTransforms{};
	std::unordered_map<ObjectType, std::unordered_map<uint, PhysicsTransformable>> currentPTransforms{};

	std::unordered_map<uchar, Ship> ships;

	void applyTransforms(std::unordered_map<ObjectType, std::unordered_map<uint, PhysicsTransformable>>& state);

	void drawAll(sf::RenderWindow& window);

	void createShip(User* const user, TeamId teamId = NO_TEAM);
	void removeShip(uchar clientId);

	ShipState getShipState(uchar clientIndex);
};
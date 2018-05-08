#include "GOManager.h"
#include <iostream>
#include "Console.h"

void GOManager::createShip(User* const user, TeamId teamId) {
	ships[user->clientId] = Ship(user, teamId);
	previousPTransforms[SHIP][user->clientId] = ships.at(user->clientId);
	currentPTransforms[SHIP][user->clientId] = ships.at(user->clientId);
}

void GOManager::removeShip(uchar clientId) {
	ships.erase(clientId);
	currentPTransforms[SHIP].erase(clientId);
}

void GOManager::drawAll(sf::RenderWindow& window) {
	for (auto& s : ships) {
		s.second.updateVisuals();
		window.draw(s.second);
	}
}

void GOManager::applyTransforms(std::unordered_map<ObjectType, std::unordered_map<uint, PhysicsTransformable>>& state) {
	for (auto& pair : state) {
		for (auto& innerPair : pair.second) {

			PhysicsTransformable* objectPTrans = nullptr;

			if (pair.first == SHIP && ships.count(innerPair.first) == 1) {
				objectPTrans = &ships[innerPair.first];
			}

			if (objectPTrans != nullptr) {
				objectPTrans->updateValues(innerPair.second);
			}
		}
	}
}

ShipState GOManager::getShipState(uchar index) {
	ShipState ss = ShipState::generateFromPTrans(ships.at(index));;
	ss.dead = ships.at(index).isDead();
	return ss;
}
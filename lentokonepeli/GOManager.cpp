#include "GOManager.h"
#include <iostream>
#include "Console.h"
#include "DefaultGun.h"
#include <Thor/Math.hpp>

void GOManager::createShip(User* user, TeamId teamId) {
	ships[user->clientId] = Ship(getUnusedPTransId(), user, teamId);
	ships[user->clientId].weapon = std::make_unique<DefaultGun>(this, user->clientId);
	addToPhysics(&ships[user->clientId]);
}

void GOManager::removeShip(sf::Uint8 clientId) {
	shipGarbage.push_back(clientId);
}

sf::Uint16 GOManager::getNewBulletId(sf::Uint8 forClientId) {
	sf::Uint16 bulletId;
	do {
		bulletId = thor::random(0, UINT16_MAX);
	} 
	while (bullets[forClientId].count(bulletId) == 1);
	return bulletId;
}

Bullet* GOManager::createBullet(sf::Uint8 forClientId, sf::Uint16 bulletId) {
	bullets[forClientId][bulletId] = Bullet(this, getUnusedPTransId(), forClientId, bulletId);
	// Don't add to physics because weapon does it after shooting
	return &(bullets[forClientId][bulletId]);
}

void GOManager::removeBullet(sf::Uint8 clientId, sf::Uint16 bulletId) {
	bulletGarbage.push_back(std::make_pair(clientId, bulletId));
}

void GOManager::drawAll(sf::RenderWindow& window) {
	for (auto& s : ships) {
		s.second.draw(window);
	}

	for (auto& pair : bullets) {
		for (auto& innerPair : pair.second) {
			innerPair.second.draw(window);
		}
	}
}

sf::Uint32 GOManager::getUnusedPTransId() {
	sf::Uint32 newId = 0;
	do {
		newId = thor::random(0, INT32_MAX);
	} 
	while (pTransPointers.count(newId) == 1);
	return newId;
}

void GOManager::addToPhysics(PhysicsTransformable* pTrans) {
	pTransPointers[pTrans->pTransId] = pTrans;
	previousPTransformsState[pTrans->pTransId] = *pTrans;
	currentPTransformsState[pTrans->pTransId] = *pTrans;
}

void GOManager::removeFromPhysics(PhysicsTransformable* pTrans) {
	pTransPointers.erase(pTrans->pTransId);
	previousPTransformsState.erase(pTrans->pTransId);
	currentPTransformsState.erase(pTrans->pTransId);
}

void GOManager::applyTransforms(std::unordered_map<sf::Uint32, PhysicsTransformable>& state) {
	for (auto& pair : state) {
		pTransPointers.at(pair.first)->updateValues(pair.second);
	}
}

ShipState GOManager::getShipState(sf::Uint8 index) {
	ShipState ss;
	ss.generateFromPTrans(ships.at(index));;
	ss.dead = ships.at(index).isDead();
	return ss;
}

void GOManager::deleteGarbage() {
	for (auto& b : bulletGarbage) {
		if (bullets[std::get<0>(b)].count(std::get<1>(b)) == 1) {
			removeFromPhysics(&bullets[std::get<0>(b)].at(std::get<1>(b)));
			bullets[std::get<0>(b)].erase(std::get<1>(b));
		}
	}
	bulletGarbage.clear();

	for (sf::Uint8 s : shipGarbage) {
		if (ships.count(s) == 1) {
			removeFromPhysics(&ships.at(s));
			ships.erase(s);
		}
	}
	shipGarbage.clear();
}

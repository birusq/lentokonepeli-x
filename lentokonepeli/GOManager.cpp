#include "GOManager.h"
#include <iostream>
#include "Console.h"
#include "DefaultGun.h"
#include <Thor/Math.hpp>

void GOManager::createShip(User* const user, TeamId teamId) {
	ships[user->clientId] = Ship(getUnusedPTransId(), user, teamId);
	ships[user->clientId].weapon = std::make_unique<DefaultGun>(this, user->clientId);
	addToPhysics(&ships[user->clientId]);
}

void GOManager::removeShip(sf::Uint8 clientId) {
	removeFromPhysics(&ships.at(clientId));
	ships.erase(clientId);
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
	removeFromPhysics(&bullets[clientId][bulletId]);
	bullets[clientId].erase(bulletId);
}

void GOManager::drawAll(sf::RenderWindow& window) {
	for (auto& s : ships) {
		s.second.draw(window);
	}

	std::vector<Bullet*> garbage;
	for (auto& pair : bullets) {
		for (auto& innerPair : pair.second) {
			Bullet* b = &innerPair.second;
			b->draw(window);
			if (b->lifeTimeCounter.getElapsedTime().asSeconds() > b->lifeTime) {
				garbage.push_back(b);
			}
		}
	}
	for (Bullet* b : garbage) {
		removeBullet(b->clientId, b->bulletId);
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
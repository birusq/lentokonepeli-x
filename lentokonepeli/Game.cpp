#include "Game.h"
#include <Thor/Vectors.hpp>
#include <iostream>
#include "PhysicsTransformable.h"
#include "Console.h"
#include "Master.h"
#include "ServerGame.h"

Game::Game() {
	goManager.init(this);
}

void Game::integrate(PhysicsTransformable& currPTrans, float dt){
	if (currPTrans.constantVelocity == false) {
		sf::Vector2f force;

		if (currPTrans.gravity)
			force += gravity;

		force += currPTrans.forceOnSelf; // Internal force eg. ship engine

		if (thor::squaredLength(currPTrans.velocity) != 0.0F)
			force += thor::squaredLength(currPTrans.velocity) * currPTrans.drag * -thor::unitVector(currPTrans.velocity); // drag

		currPTrans.acceleration = force; // f/m when m = 1
		currPTrans.velocity += currPTrans.acceleration * dt;
	}
	currPTrans.move(currPTrans.velocity * dt); 
	currPTrans.rotate(currPTrans.angularVelocity * dt);
}

void Game::improveHandling(Ship& ship) {
	PhysicsTransformable& target = goManager.currentPTransformsState[ship.pTransId];

	float velocityMagnitude = thor::length(target.velocity);

	if (velocityMagnitude != 0) {

		float angle = thor::signedAngle(target.getRotationVector(), target.velocity);

		float factor = (-0.00012F * angle * angle + 1.0F) * 0.08F;
		if (factor < 0.0F)
			factor = 0.0F;

		target.velocity = target.getRotationVector() * velocityMagnitude * factor + target.velocity * (1.0F - factor);
	}
}

void Game::collisionDetectAll(std::unordered_map<Team::Id, Team>& teams) {
	auto it1 = teams.begin();
	auto it2 = std::next(it1, 1);
	for (it1; it1 != teams.end(); it1++) {

		//console::stream << "team " << (int)it1->first << ": members " << (int)it1->second.members.size();
		//console::dlogStream();

		for (it2; it2 != teams.end(); it2++) {

			for (sf::Uint8& t1Client : it1->second.members) {

				for (sf::Uint8& t2Client : it2->second.members) {


					// Bullet collisions
					for (auto& pair : goManager.bullets[t1Client]) {
						if (pair.second->collidesWith(level.spawnPointColliders.at(it2->first))) {
							// bullet destroys itself on collision
						}
						if (pair.second->collidesWith(goManager.ships.at(t2Client))) {
							onBulletCollision(*pair.second, goManager.ships[t2Client]);
						}
					}
					for (auto& pair : goManager.bullets[t2Client]) {
						if (pair.second->collidesWith(level.spawnPointColliders.at(it1->first))) {
							// bullet destroys itself on collision
						}
						if (pair.second->collidesWith(goManager.ships.at(t1Client))) {
							onBulletCollision(*pair.second, goManager.ships[t1Client]);
						}
					}

					// Player on player collisions
					if (goManager.ships.at(t1Client).collidesWith(goManager.ships.at(t2Client))) {
						onShipToShipCollision(goManager.ships[t1Client], goManager.ships[t2Client]);
					}
				}
			}

		}
		it2 = std::next(it1, 1);
		if (it2 != teams.end())
			it2++;
	}

	// Check level collisions
	for (auto& team : teams) {
		for (sf::Uint8& clientId : team.second.members) {
			for (auto& pair : goManager.bullets[clientId]) {
				//TODO: Level collisions, probably just die when touching
			}
		}
	}
}

void Game::updateAllHitboxPositions() {
	for(auto& pair : goManager.ships) {
		pair.second.updateHitbox();
	}
	for(auto& pair : goManager.bullets) {
		for(auto& innerPair : pair.second) {
			innerPair.second->updateHitbox();
		}
	}
}

void Game::handleSpawnTimers(float dt) {
	for (auto it = spawnTimers.begin(); it != spawnTimers.end();) {
		if (it->second.getTimeRemaining() <= 0.0F) {
			spawnShip(it->first);
			it = spawnTimers.erase(it);
		}
		else {
			it++;
		}
	}
}

void Game::resetShipTransform(Ship & ship) {
	// Reset all transforms
	PhysicsTransformable& cpTrans = goManager.currentPTransformsState.at(ship.pTransId);
	cpTrans.setPosition(level.spawnPoints[ship.owner->teamId]);
	cpTrans.setRotation(0);
	cpTrans.setToRest();

	goManager.previousPTransformsState.at(ship.pTransId) = cpTrans;
}

void Game::quit() {
	onQuit();
	running = false;
}



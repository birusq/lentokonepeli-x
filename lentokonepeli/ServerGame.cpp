#include "ServerGame.h"
#include "Master.h"
#include <iostream>
#include "Server.h"
#include "Ship.h"
#include "Weapon.h"
#include "Damageable.h"

ServerGame::ServerGame() {
	server.init(this);
	server.start(30);
}

void ServerGame::loop() {
	float dt = 1.0F / fixedUpdateFps;

	sf::Clock clock;

	int renderCounter = 0;

	while (master->window.isOpen() && running) {

		sf::Event event;
		while (master->window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				master->quit();
			}
			master->gui.handleEvent(event);
		}
		
		float frameTime = clock.restart().asSeconds();
		updateFrameTimes(frameTime);

		fixedUpdate(dt);

		renderCounter++;
		if (renderCounter == 2) {
			render(master->window, dt * 2.0F);
			renderCounter = 0;
		}

		sf::sleep(sf::seconds(dt - clock.getElapsedTime().asSeconds()));
	}
}

void ServerGame::onUserConnect(User* const user) {
	goManager.createShip(user, user->teamId);
}

void ServerGame::onUserDisconnect(sf::Uint8 clientId) {
	goManager.removeShip(clientId);
}

void ServerGame::onClientJoinTeam(sf::Uint8 clientId, Team::Id newTeam) {
	// Nothing yet
}

void ServerGame::onSpawnRequest(sf::Uint8 clientId) {
	if (goManager.ships.count(clientId) == 1) {
		if (goManager.ships[clientId].isDead()) {
			float spawnTime = level.respawnTime - goManager.ships[clientId].timeSinceDeath.getElapsedTime().asSeconds();
			server.sendShipSpawn(clientId, true, spawnTime);
			if (spawnTime > 0.0F) {
				spawnTimers[clientId].start(spawnTime);
			}
			else {
				spawnShip(clientId);
			}
		}
	}
}

void ServerGame::fillShipInit(User& user, ShipInitMessage& shipInitMsg) {
	Ship& ship = goManager.ships.at(user.clientId);
	shipInitMsg.clientId = user.clientId;
	shipInitMsg.health = ship.health;
}

void ServerGame::onShipDeath(Ship* ship) {
	KillDetails killDetails;
	killDetails.clientKilled = ship->owner->clientId;
	killDetails.contributors = ship->dmgContributors;
	scoreBoard.addKillDetails(killDetails);

	server.broadcastKillDetails(killDetails);
}

void ServerGame::spawnShip(sf::Uint8 clientId) {
	Ship& ship = goManager.ships.at(clientId);

	ship.respawn();

	// Reset all transforms
	PhysicsTransformable& ppTrans = goManager.previousPTransformsState.at(ship.pTransId);
	ppTrans.setPosition(level.spawnPoints[server.users.at(clientId).teamId]);
	ppTrans.setRotation(0);
	goManager.currentPTransformsState.at(ship.pTransId) = ppTrans;
}

void ServerGame::render(sf::RenderWindow& window, float dt) {
	window.clear(sf::Color(62, 65, 71));

	master->gui.draw(dt);

	window.display();
}

void ServerGame::fixedUpdate(float dt) {
	goManager.previousPTransformsState = goManager.currentPTransformsState;
	
	server.update();

	ServerShipStates serverStates;
	
	handleSpawnTimers(dt);

	for (auto& pair : server.shipStateJitterBuffers) {
		if (pair.second.size() > 0) {
			serverStates.states[pair.first] = pair.second.front();
			pair.second.pop_front();
		}
	}

	applyClientShipStates(serverStates, dt);

	for (auto& pair : goManager.bullets) {
		for (auto& innerPair : pair.second) {
			integrate(goManager.currentPTransformsState.at(innerPair.second->pTransId), dt);
		}
	}

	goManager.applyTransforms(goManager.currentPTransformsState);

	collisionDetectAll(server.teams);

	goManager.deleteGarbage();

	updateServerStates(serverStates);

	server.broadcastShipStates(serverStates);
}

void ServerGame::applyClientShipStates(ServerShipStates& sss, float dt) {
	for (auto& pair : server.users) {
		sf::Uint8 clientId = pair.first;

		if (sss.states.count(clientId) == 1) {
			ShipState& shipState = sss.states[clientId];
			Ship& ship = goManager.ships.at(clientId);

			if (ship.isDead() == false) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));
				ship.setWeaponTrans(shipState.position, shipState.rotation);
				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
				}
			}
		}
		else {
			integrate(goManager.ships.at(clientId), dt);
		}
	}
}

void ServerGame::updateServerStates(ServerShipStates& sss) {
	for (auto& pair : goManager.ships) {
		sss.states[pair.first].generateFromPTrans(pair.second);
		sss.states[pair.first].dead = pair.second.isDead();
	}
}

void ServerGame::onBulletCollision(Bullet& bullet, Ship& targetShip) {
	console::stream << server.users.at(bullet.clientId).username.C_String() << " hit " << targetShip.owner->username.C_String() << " with a bullet";
	console::dlogStream();
	targetShip.takeDmg(bullet.damage, Damageable::DMG_BULLET, bullet.clientId);
	server.sendBulletHitShip(&bullet, &targetShip);
}

void ServerGame::onShipCollision(Ship& ship1, Ship& ship2) {
	console::dlog(std::string(ship1.owner->username.C_String()) + " collided with " + std::string(ship2.owner->username.C_String()));
	bool s1Immune = ship1.bodyHitImmunityTimer.isDone() == false;
	bool s2Immune = ship2.bodyHitImmunityTimer.isDone() == false;
	if (s1Immune == false) {
		ship1.takeDmg(ship2.bodyHitDamage, Damageable::DMG_SHIP_COLLISION, ship2.owner->clientId);
	}
	if (s2Immune == false) {
		ship2.takeDmg(ship1.bodyHitDamage, Damageable::DMG_SHIP_COLLISION, ship1.owner->clientId);
	}
	server.sendShipsCollided(&ship1, s1Immune, &ship2, s2Immune);
}

#include "ServerGame.h"
#include "Master.h"
#include <iostream>
#include "Server.h"
#include "Ship.h"
#include "Weapon.h"

ServerGame::ServerGame() {
	server.init(this);
	server.start(10);
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
			render(master->window);
			renderCounter = 0;
		}

		sf::sleep(sf::seconds(dt - clock.getElapsedTime().asSeconds()));
	}
}

void ServerGame::onUserConnect(User* const user) {
	goManager.createShip(user);
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
				spawnTimers[clientId] = spawnTime;
			}
		}
	}
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

void ServerGame::render(sf::RenderWindow& window) {
	window.clear(sf::Color(62, 65, 71));

	master->gui.draw();

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

	// TODO: and if dead then send time until respawn to client

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
	targetShip.takeDmg(bullet.damage);
	server.sendBulletHitShip(&bullet, &targetShip);
}

void ServerGame::onShipCollision(Ship& ship1, Ship& ship2) {
	console::dlog(std::string(ship1.owner->username.C_String()) + " collided with " + std::string(ship2.owner->username.C_String()));
	ship1.takeDmg(ship2.bodyHitDamage);
	ship2.takeDmg(ship1.bodyHitDamage);
	server.sendShipsCollided(&ship1, &ship2);
}

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

void ServerGame::render(sf::RenderWindow& window) {
	window.clear(sf::Color(62, 65, 71));

	master->gui.draw();

	window.display();
}

void ServerGame::fixedUpdate(float dt) {
	goManager.previousPTransformsState = goManager.currentPTransformsState;
	
	server.update();

	ServerShipStates serverStates;
	
	// TODO: integrate ships when we don't receive a movement packet

	for (auto& pair : server.shipStateJitterBuffers) {
		if (pair.second.size() > 0) {
			serverStates.states[pair.first] = pair.second.front();
			pair.second.pop_front();
		}
	}

	applyClientShipStates(serverStates);

	for (auto& pair : goManager.bullets) {
		for (auto& innerPair : pair.second) {
			integrate(goManager.currentPTransformsState.at(innerPair.second.pTransId), dt);
		}
	}

	goManager.applyTransforms(goManager.currentPTransformsState);

	collisionDetectAll(server.teams);

	goManager.deleteGarbage();

	updateServerStates(serverStates);

	// TODO: and if dead then send time until respawn to client

	server.broadcastShipStates(serverStates);
}

void ServerGame::applyClientShipStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {
		
		ShipState& shipState = pair.second;
		sf::Uint8 clientId = pair.first;

		if (server.users.count(clientId) == 1) {

			Ship& ship = goManager.ships.at(clientId);

			if (ship.isDead() == false) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));
				ship.setWeaponTrans(shipState.position, shipState.rotation);
				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
				}
			}
			
			if (ship.isDead() == true && shipState.dead == false && ship.timeSinceDeath.getElapsedTime().asSeconds() > 0.25F) {
				ship.respawn();
			}
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

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

void ServerGame::onClientJoinTeam(sf::Uint8 clientId, TeamId newTeam) {
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

	collisionDetectAll();

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

void ServerGame::collisionDetectAll() {
	// Update all hitbox positions
	for (auto& pair : goManager.ships) {
		pair.second.updateHitbox();
	}
	for (auto& pair : goManager.bullets) {
		for (auto& innerPair : pair.second) {
			innerPair.second.updateHitbox();
		}
	}

	// Check collision (check all variations of teams)
	auto it1 = server.teams.begin();
	auto it2 = std::next(it1, 1);
	for (it1; it1 != server.teams.end(); it1++) {
		for (it2; it2 != server.teams.end(); it2++) {

			for (sf::Uint8& t1Client : it1->second.members) {
				for (sf::Uint8& t2Client : it2->second.members) {

					// Bullet collisions
					for (auto& pair : goManager.bullets[t1Client]) {
						if (pair.second.collidesWith(goManager.ships.at(t2Client))) {
							console::stream << server.users.at(t1Client).username.C_String() << " hit " << server.users.at(t2Client).username.C_String() << " with a bullet";
							console::dlogStream();
							goManager.ships.at(t2Client).takeDmg(pair.second.damage);
							server.sendBulletHitShip(&pair.second, &goManager.ships.at(t2Client));
						}
					}
					for (auto& pair : goManager.bullets[t2Client]) {
						if (pair.second.collidesWith(goManager.ships.at(t1Client))) {
							console::stream << server.users.at(t1Client).username.C_String() << " hit " << server.users.at(t2Client).username.C_String() << " with a bullet";
							console::dlogStream();
							goManager.ships.at(t1Client).takeDmg(pair.second.damage);
							server.sendBulletHitShip(&pair.second, &goManager.ships.at(t1Client));
						}
					}


					// Player collisions
					if (goManager.ships.at(t1Client).collidesWith(goManager.ships.at(t2Client))) {
						console::dlog(std::string(server.users.at(t1Client).username.C_String()) + " collided with " + std::string(server.users.at(t2Client).username.C_String()));
						Ship& ship1 = goManager.ships.at(t1Client);
						Ship& ship2 = goManager.ships.at(t2Client);
						ship1.takeDmg(ship2.bodyHitDamage);
						ship2.takeDmg(ship1.bodyHitDamage);
						server.sendShipsCollided(&ship1, &ship2);
					}
				}
			}

		}
		it2 = std::next(it1, 1);
		if (it2 != server.teams.end())
			it2++;
	}
}

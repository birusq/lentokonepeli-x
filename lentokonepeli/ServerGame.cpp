#include "ServerGame.h"
#include "Master.h"
#include "Server.h"
#include "Ship.h"
#include "Weapon.h"
#include "Damageable.h"

ServerGame::ServerGame() {
	server.init(this);
	server.start(30);

	scores.init(&server);

	isserver = true;
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

void ServerGame::onUserConnect(User& user) {
	goManager.createShip(user, user.teamId);
	//Don't add user to scores yet because they have no team yet
}

void ServerGame::beforeUserDisconnect(User& user) {
	scores.deleteUser(user);
	goManager.removeShip(user.clientId);
}

void ServerGame::onClientJoinTeam(sf::Uint8 clientId, Team::Id oldTeam, Team::Id newTeam) {
	scores.addUser(server.users[clientId]);
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
	shipInitMsg.dead = ship.isDead();
	shipInitMsg.health = ship.health;
}

void ServerGame::onShipDeath(Ship* ship) {
	KillDetails killDetails;
	killDetails.clientKilled = ship->owner->clientId;
	killDetails.contributors = ship->dmgContributors;
	scores.addKillDetails(killDetails);

	server.broadcastKillDetails(killDetails);

	resetShipTransform(*ship);
}

void ServerGame::spawnShip(sf::Uint8 clientId) {
	Ship& ship = goManager.ships.at(clientId);
	resetShipTransform(ship);
	ship.respawn();
}

void ServerGame::shipsOutsideBoundsCheck() {
	//Check level bounds
	for(auto& pair : goManager.ships) {
		if(pair.second.hitboxDisabled == false) {

			sf::Vector2f pos = pair.second.hitbox.getPosition();

			if(pos.x < level.borderWidth || pos.x > level.width - level.borderWidth ||
				pos.y < level.borderWidth || pos.y > level.height - level.borderWidth) {
				onShipToGroundCollision(pair.second);
			}
		}
	}
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
			if(pair.second.back().second != nullptr) {
				serverStates.states[pair.first] = *pair.second.back().second;
			}
			pair.second.pop_back();
		}
	}

	applyClientShipStates(serverStates, dt);

	for (auto& pair : goManager.bullets) {
		for (auto& innerPair : pair.second) {
			integrate(goManager.currentPTransformsState.at(innerPair.second->pTransId), dt);
		}
	}

	goManager.applyTransforms(goManager.currentPTransformsState);

	updateAllHitboxPositions();
	collisionDetectAll(server.teams);
	shipsOutsideBoundsCheck();

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

			if (!shipState.dead && !ship.isDead()) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));
				ship.setWeaponTrans(shipState.position, shipState.rotation);
				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
				}
				ship.throttle = shipState.throttle;
			}
			else {
				ship.setPosition(level.center);
			}
		}
		else {
			integrate(goManager.currentPTransformsState.at(goManager.ships.at(clientId).pTransId), dt);
		}
	}
}

void ServerGame::updateServerStates(ServerShipStates& sss) {
	for(auto& pair : goManager.ships) {
		sss.states[pair.first].generateFromPTrans(pair.second);
		sss.states[pair.first].throttle = pair.second.throttle;
		sss.states[pair.first].dead = pair.second.isDead();
	}
}

void ServerGame::onBulletCollision(Bullet& bullet, Ship& targetShip) {
	console::stream << server.users.at(bullet.clientId).username.C_String() << " hit " << targetShip.owner->username.C_String() << " with a bullet";
	console::dlogStream();
	targetShip.takeDmg(bullet.damage, Damageable::DMG_BULLET, bullet.clientId);
	
	DamageMessage dmg(bullet.clientId, bullet.damage, targetShip.owner->clientId, Damageable::DamageType::DMG_BULLET);
	server.sendDamage(dmg);
}

void ServerGame::onShipToShipCollision(Ship& ship1, Ship& ship2) {
	console::dlog(std::string(ship1.owner->username.C_String()) + " collided with " + std::string(ship2.owner->username.C_String()));
	if (ship1.bodyHitImmunityTimer.isDone() == true) {
		ship1.takeDmg(ship2.bodyHitDamage, Damageable::DMG_SHIP_COLLISION, ship2.owner->clientId);
		DamageMessage dmg(ship2.owner->clientId, ship2.bodyHitDamage, ship1.owner->clientId, Damageable::DamageType::DMG_SHIP_COLLISION);
		server.sendDamage(dmg);
	}
	if (ship2.bodyHitImmunityTimer.isDone() == true) {
		ship2.takeDmg(ship1.bodyHitDamage, Damageable::DMG_SHIP_COLLISION, ship1.owner->clientId);
		DamageMessage dmg(ship1.owner->clientId, ship1.bodyHitDamage, ship2.owner->clientId, Damageable::DamageType::DMG_SHIP_COLLISION);
		server.sendDamage(dmg);
	}
}

void ServerGame::onShipToGroundCollision(Ship& ship) {
	console::stream << server.users.at(ship.owner->clientId).username.C_String() << " hit ground";
	console::dlogStream();
	DamageMessage dmg(ship.owner->clientId, UINT16_MAX, ship.owner->clientId, Damageable::DamageType::DMG_GROUND_COLLISION);
	ship.takeDmg((int)dmg.damage, dmg.damageType);
	server.sendDamage(dmg);
}

void ServerGame::onQuit() {
	server.close();
}

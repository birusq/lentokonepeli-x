#include "ClientGame.h" 
#include <iostream>
#include "Raknet\RakWString.h"
#include <Thor/Vectors.hpp>
#include "User.h"
#include "Weapon.h"
#include "Master.h"

ClientGame::ClientGame(std::string hostIp_) : hostIp{ hostIp_ } {

	client.init(this);

	if (master->settings.username.getValue() == "*") {
		client.start(hostIp_, master->settings.tempUsername.c_str());
	}
	else {
		client.start(hostIp_, master->settings.username.getValue().c_str());
	}
}

void ClientGame::onOtherUserConnect(User* const user) {
	goManager.createShip(user, user->teamId);
}

void ClientGame::onOtherUserDisconnect(sf::Uint8 clientId) {
	goManager.removeShip(clientId);
}

void ClientGame::onTeamJoin(sf::Uint8 clientId, TeamId newTeam) {
	std::string message;
	if (clientId == client.myId) {
		message += "You have";
		master->gui.teamJoinAccepted();
	}
	else {
		message += client.users.at(clientId).username + " has";
	}
	message += " joined";
	switch (newTeam) {
	case RED_TEAM:
		message += " the red team";
		break;
	case BLUE_TEAM:
		message += " the blue team";
		break;
	}
	if (newTeam != NO_TEAM)
		console::log(message);

	goManager.ships.at(clientId).assignTeam(newTeam);
}

void ClientGame::onBulletHit(BulletDamage& bDmg) {
	if (goManager.ships.count(bDmg.shooterId) == 1 && goManager.bullets[bDmg.shooterId].count(bDmg.bulletId) == 1 && goManager.ships.count(bDmg.targetId) == 1) {
		goManager.bullets[bDmg.shooterId][bDmg.bulletId].lifeTime = bDmg.bulletLifetime;
		Ship& targetShip = goManager.ships[bDmg.targetId];
		targetShip.takeDmg(bDmg.damage);

		fixHealthDesyncIfNeeded(targetShip, bDmg.newHealth);
	}
}

void ClientGame::onShipsCollision(ShipsCollisionDamage & scDmg) {
	if (goManager.ships.count(scDmg.clientId1) == 1 && goManager.ships.count(scDmg.clientId2) == 1) {
		Ship& ship1 = goManager.ships[scDmg.clientId1];
		Ship& ship2 = goManager.ships[scDmg.clientId2];

		ship1.takeDmg(scDmg.dmgTo1);
		ship2.takeDmg(scDmg.dmgTo2);

		fixHealthDesyncIfNeeded(ship1, scDmg.newHealth1);
		fixHealthDesyncIfNeeded(ship2, scDmg.newHealth2);
	}
}

void ClientGame::fixHealthDesyncIfNeeded(Damageable & target, int newHealth) {
	if (target.health > newHealth) {
		target.takeDmg(target.health - newHealth);
	}
	else if (target.health < newHealth) {
		target.restoreHealth(newHealth - target.health);
	}
}

void ClientGame::onConnectionComplete() {
	goManager.createShip(client.myUser());
	goManager.ships.at(client.myId).localPlayer = true;
}

void ClientGame::respawnMyShip() {
	inputDisabledTimer.restart();
	Ship& myShip = goManager.ships.at(client.myId);
	myShip.respawn();
	goManager.previousPTransformsState.at(myShip.pTransId).setPosition(level.spawnPoints[client.myUser()->teamId]);
	goManager.currentPTransformsState.at(myShip.pTransId).setPosition(level.spawnPoints[client.myUser()->teamId]);
}

void ClientGame::render(sf::RenderWindow& window, GOManager& goManager) {
	window.clear(sf::Color(50, 50, 50));

	window.draw(level);

	goManager.drawAll(window);

	if (client.connectionDone && goManager.ships.count(client.myId) == 1) {
		sf::View view = window.getView();
		view.setCenter(goManager.ships[client.myId].getPosition());
		window.setView(view);
	}

	master->gui.lastPing = client.lastPing;
	master->gui.draw();

	window.display();
}

void ClientGame::loop() {
	float t = 0.0;
	float dt = 1.0F / fixedUpdateFps;

	sf::Clock clock;
	float accumulator = 0.0;

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

		if (frameTime > 0.25)
			frameTime = 0.25;

		accumulator += frameTime;

		while (accumulator >= dt)
		{
			fixedUpdate(dt);

			t += dt;
			accumulator -= dt;
		}

		const float alpha = accumulator / dt;

		update(frameTime, alpha);

		render(master->window, goManager);
	}
}

void ClientGame::fixedUpdate(float dt) {
	client.update();
	
	if (client.connectionDone) {

		goManager.previousPTransformsState = goManager.currentPTransformsState;

		// Input
		Input input;
		int bulletId = -1;
		if (master->window.hasFocus()) {
			input = processInput();
			bulletId = applyInput(input, goManager.ships.at(client.myId), dt);
		}

		// Update my game with server states
		if (client.serverStateJitterBuffer.size() > 0) {
			applyServerStates(client.serverStateJitterBuffer.front());
			client.serverStateJitterBuffer.pop_front();
		}

		// Integration
		for (auto& pair : goManager.currentPTransformsState) {
			// Is ship is dead, dont integrate
			if (Ship* s = dynamic_cast<Ship*>(goManager.pTransPointers.at(pair.second.pTransId))) {
				if (s->isDead()) {
					continue;
				}
			}

			integrate(pair.second, dt);
		}

		//Send my state to server
		ShipState ss = goManager.getShipState(client.myId);
		ss.throttle = input.moveForward;
		if (bulletId != -1) {
			ss.shoot = true;
			ss.bulletId = sf::Uint16(bulletId);
		}
		
		client.sendShipUpdate(ss);

		goManager.deleteGarbage();
	}
}

void ClientGame::applyServerStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {

		ShipState& shipState = pair.second;
		sf::Uint8 clientId = pair.first;

		if (client.users.count(clientId) == 1) {

			Ship& ship = goManager.ships.at(clientId);

			if (clientId != client.myId) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));

				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
				}

				if (shipState.throttle)
					ship.throttle = true;
				else
					ship.throttle = false;

				if (ship.isDead() == true && shipState.dead == false && ship.timeSinceDeath.getElapsedTime().asSeconds() > 0.25F) {
					ship.respawn();
				}
			}
		}
	}
}

void ClientGame::update(float frameTime, float alpha) {
	auto state = goManager.currentPTransformsState;
	for (auto& pair : state) {
		pair.second = PhysicsTransformable::lerp(goManager.previousPTransformsState.at(pair.first), goManager.currentPTransformsState.at(pair.first), alpha);
	}

	goManager.applyTransforms(state);
}

Input ClientGame::processInput() {
	Input input;
	if (inputDisabledTimer.getElapsedTime().asSeconds() > inputDisabledTime) {
		if (sf::Keyboard::isKeyPressed(master->settings.moveForwardKey)) {
			input.moveForward = true;
		}
		if (sf::Keyboard::isKeyPressed(master->settings.turnLeftKey)) {
			input.turnLeft = true;
		}
		if (sf::Keyboard::isKeyPressed(master->settings.turnRightKey)) {
			input.turnRight = true;
		}
		if (sf::Keyboard::isKeyPressed(master->settings.shootKey)) {
			input.shooting = true;
		}
	}
	return input;
}

int ClientGame::applyInput(Input input, Ship& ship, float dt) {
	
	if (ship.isDead() && input.any()) {
		respawnMyShip();
	}
	
	PhysicsTransformable& currTrans = goManager.currentPTransformsState[ship.pTransId];
	if (input.moveForward) {
		currTrans.forceOnSelf = currTrans.getRotationVector() * 30.0F;
		improveHandling(ship);
		ship.throttle = true;
	}
	else {
		currTrans.forceOnSelf *= 0.0F;
		ship.throttle = false;
	}

	if (input.turnLeft && input.turnRight) {
		if (currTrans.angularVelocity >= 1.0F) {
			currTrans.angularVelocity -= ship.turnSpeed / ship.turnSmoothingFrames;
		}
		else if (currTrans.angularVelocity <= -1.0F){
			currTrans.angularVelocity += ship.turnSpeed / ship.turnSmoothingFrames;
		}
	}
	else if (input.turnLeft) {
		currTrans.angularVelocity -= ship.turnSpeed / ship.turnSmoothingFrames;
		if (currTrans.angularVelocity < -ship.turnSpeed) {
			currTrans.angularVelocity = -ship.turnSpeed;
		}
	}
	else if (input.turnRight) {
		currTrans.angularVelocity += ship.turnSpeed / ship.turnSmoothingFrames;
		if (currTrans.angularVelocity > ship.turnSpeed) {
			currTrans.angularVelocity = ship.turnSpeed;
		}
	}
	else {
		if (currTrans.angularVelocity >= 1.0F) {
			currTrans.angularVelocity -= ship.turnSpeed / ship.turnSmoothingFrames;
		}
		else if (currTrans.angularVelocity <= -1.0F) {
			currTrans.angularVelocity += ship.turnSpeed / ship.turnSmoothingFrames;
		}
	}

	if (input.shooting) {
		return ship.weapon->shoot(); // shoots only if firerate allows
	}
	return -1;
}
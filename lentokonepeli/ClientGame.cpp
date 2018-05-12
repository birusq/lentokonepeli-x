#include "ClientGame.h" 
#include <iostream>
#include "Raknet\RakWString.h"
#include <Thor/Vectors.hpp>

ClientGame::ClientGame(Master* master_, std::string hostIp_) : Game(master_), hostIp{ hostIp_ } {

	client.init(master_, this);

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
		gui->teamJoinAccepted();
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

void ClientGame::onBulletHit(sf::Uint8 shooterId, sf::Uint16 bulletId, sf::Uint8 targetId, sf::Uint16 damage) {
	if (goManager.ships.count(shooterId) == 1 && goManager.bullets.at(shooterId).count(bulletId) == 1 && goManager.ships.count(targetId) == 1) {
		goManager.removeBullet(shooterId, bulletId);
		goManager.ships.at(targetId).takeDmg(damage);
	}
}

void ClientGame::onConnectionComplete() {
	goManager.createShip(client.myUser());
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

	gui->lastPing = client.lastPing;
	gui->draw();

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
			gui->handleEvent(event);
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
	}
	else {
		currTrans.forceOnSelf *= 0.0F;
	}

	if (input.turnLeft && input.turnRight) {
		currTrans.angularVelocity = 0.0F;
	}
	else if (input.turnLeft) {
		currTrans.angularVelocity = -180.0F;
	}
	else if (input.turnRight) {
		currTrans.angularVelocity = 180.0F;
	}
	else {
		currTrans.angularVelocity = 0.0F;
	}

	if (input.shooting) {
		return ship.weapon->shoot(); // shoots only if firerate allows
	}
	return -1;
}
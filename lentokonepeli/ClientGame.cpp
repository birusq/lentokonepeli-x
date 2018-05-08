#include "ClientGame.h" 
#include <iostream>
#include "Raknet\RakWString.h"

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
	goManager.createShip(user);
}

void ClientGame::onOtherUserDisconnect(uchar clientId) {
	goManager.removeShip(clientId);
}


void ClientGame::onConnectionComplete() {
	goManager.createShip(client.myUser());
}

void ClientGame::onSpawnAllowed() {
	Ship& myShip = goManager.ships.at(client.myId);
	myShip.setHealthToFull();
	goManager.currentPTransforms[SHIP].at(client.myId).setPosition(sf::Vector2f(100, 100));
}

void ClientGame::render(sf::RenderWindow& window, GOManager& goManager) {
	window.clear(sf::Color(85, 121, 178, 255));

	window.draw(level);

	goManager.drawAll(window);

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

		goManager.previousPTransforms = goManager.currentPTransforms;

		// Input
		Input input;
		if (master->window.hasFocus()) {
			input = processInput();
			applyInput(input, goManager.currentPTransforms[SHIP].at(client.myId), dt);
		}

		// Update my game with server states
		if (client.serverStateJitterBuffer.size() > 0) {
			applyServerStates(client.serverStateJitterBuffer.front());
			client.serverStateJitterBuffer.pop_front();
		}

		// Integration
		for (auto& pair : goManager.currentPTransforms) {
			for (auto& innerPair : pair.second) {
				if (pair.first == SHIP && goManager.ships.at(innerPair.first).isDead())
					continue;

				integrate(innerPair.second, dt);
			}
		}

		//Send my state to server
		ShipState ss = goManager.getShipState(client.myId);
		client.sendShipUpdate(ss);
	}
}

void ClientGame::applyServerStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {

		uchar clientId = pair.first;

		if (clientId != client.myId) {
			ShipState::applyToPTrans(pair.second, goManager.currentPTransforms[SHIP][clientId]);
		}

		if (goManager.ships[clientId].isDead() == true && pair.second.dead == false) {
			goManager.ships[clientId].setHealthToFull();
		}
	}
}

void ClientGame::update(float frameTime, float alpha) {
	auto state = goManager.currentPTransforms;
	for (auto& pair : state) {
		for (auto& innerPair : pair.second) {
			innerPair.second = PhysicsTransformable::lerp(goManager.previousPTransforms[pair.first][innerPair.first], goManager.currentPTransforms[pair.first][innerPair.first], alpha);
		}
	}

	goManager.applyTransforms(state);
}

Input ClientGame::processInput() {
	Input input;
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
		input.turnRight = true;
	}
	return input;
}

void ClientGame::applyInput(Input input, PhysicsTransformable& controlTarget, float dt) {
	if (input.moveForward) {
		controlTarget.forceOnSelf = controlTarget.getRotationVector() * 40.0F;
	}
	else {
		controlTarget.forceOnSelf *= 0.0F;
	}

	if (input.turnLeft && input.turnRight) {
		controlTarget.angularVelocity = 0.0F;
	}
	else if (input.turnLeft) {
		controlTarget.angularVelocity = -250.0F;
	}
	else if (input.turnRight) {
		controlTarget.angularVelocity = 250.0F;
	}
	else {
		controlTarget.angularVelocity = 0.0F;
	}

	if (input.shoot) {
		// TODO
	}
}
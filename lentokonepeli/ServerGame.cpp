#include "ServerGame.h"
#include "Master.h"
#include <iostream>

ServerGame::ServerGame(Master* master_) : Game(master_) {
	server.init(master_, this);
	server.start(4);
}

void ServerGame::loop() {
	float t = 0.0;
	float dt = 1.0F / fixedUpdateFps;

	sf::Clock clock;

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

		fixedUpdate(dt);

		render(master->window);

		sf::sleep(sf::seconds(dt - clock.getElapsedTime().asSeconds()));
	}
}

void ServerGame::onUserConnect(User* const user) {
	goManager.createShip(user);
}

void ServerGame::onUserDisconnect(uchar clientId) {
	goManager.removeShip(clientId);
}

void ServerGame::onClientJoinTeam(uchar clientId, TeamId newTeam) {
	if (newTeam != NO_TEAM)
		server.sendAllowSpawnMsg(clientId);
}

void ServerGame::render(sf::RenderWindow& window) {
	window.clear(sf::Color(62, 65, 71));

	gui->draw();

	window.display();
}

void ServerGame::fixedUpdate(float dt) {
	goManager.previousPTransforms = goManager.currentPTransforms;
	
	server.update();

	ServerShipStates serverStates;
	
	// TODO: integrate ships when we don't receive a movement packet
	for (auto& pair : server.shipStateJitterBuffers) {
		if (pair.second.size() > 0) {
			serverStates.states[pair.first] = pair.second.front();
			pair.second.pop_front();
		}
	}
		
	applyServerStates(serverStates);

	// TODO: integrate non player controlled objects, eg. bullets

	goManager.applyTransforms(goManager.currentPTransforms);

	// TODO: Check collisions (bullet hits)

	ServerShipStates newServerStates = getServerStatesFromShips();
	// TODO: need to set firing tag to new states (not done above ^)

	// TODO: and if dead then send time until respawn to client

	server.broadcastShipStates(newServerStates);
}

void ServerGame::applyServerStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {
		uchar clientId = pair.first;

		ShipState::applyToPTrans(pair.second, goManager.currentPTransforms[SHIP][clientId]);

		// TODO: check respawning timer
		if (goManager.ships[clientId].isDead() == true && pair.second.dead == false) {
			
			goManager.ships[clientId].setHealthToFull();
		}

		// TODO: bullets
	}
}

ServerShipStates ServerGame::getServerStatesFromShips() {
	ServerShipStates sss;
	for (auto& pair : goManager.ships) {

		Ship& ship = pair.second;
		ShipState shipState = ShipState::generateFromPTrans(ship);
		shipState.dead = ship.isDead();

		sss.states[pair.first] = shipState;
	}
	return sss;
}




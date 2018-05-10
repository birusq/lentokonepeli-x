#include "ServerGame.h"
#include "Master.h"
#include <iostream>

ServerGame::ServerGame(Master* master_) : Game(master_) {
	server.init(master_, this);
	server.start(4);
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
			gui->handleEvent(event);
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
	if (newTeam != NO_TEAM)
		server.sendAllowSpawnMsg(clientId);
}

void ServerGame::render(sf::RenderWindow& window) {
	window.clear(sf::Color(62, 65, 71));

	gui->draw();

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
		
	applyServerStates(serverStates);

	// TODO: integrate non player controlled objects, eg. bullets

	goManager.applyTransforms(goManager.currentPTransformsState);

	// TODO: Check collisions (bullet hits)

	ServerShipStates newServerStates = getServerStatesFromShips();
	// TODO: need to set firing tag to new states (not done above ^)

	// TODO: and if dead then send time until respawn to client

	server.broadcastShipStates(newServerStates);
}

void ServerGame::applyServerStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {
		sf::Uint8 clientId = pair.first;

		pair.second.applyToPTrans(goManager.currentPTransformsState[goManager.ships[clientId].pTransId]);

		// TODO: check respawning timer
		if (goManager.ships[clientId].isDead() == true && pair.second.dead == false) {
			
			goManager.ships[clientId].setHealthToFull();
		}
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




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

	
	sf::View view = master->window.getView();
	view.setCenter(level.center);
	master->window.setView(view);
}

void ClientGame::onOtherUserConnect(User* const user) {
	goManager.createShip(user, user->teamId);
}

void ClientGame::onOtherUserDisconnect(sf::Uint8 clientId) {
	goManager.removeShip(clientId);
}

void ClientGame::onTeamJoin(sf::Uint8 clientId, Team::Id newTeam) {
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
	case Team::RED_TEAM:
		message += " the red team";
		break;
	case Team::BLUE_TEAM:
		message += " the blue team";
		break;
	}
	if (newTeam != Team::NO_TEAM)
		console::log(message);

	goManager.ships.at(clientId).assignTeam(newTeam);
}

void ClientGame::onDamage(DamageMessage& dmg) {
	if (goManager.ships.count(dmg.dealerId) == 1 && goManager.ships.count(dmg.targetId) == 1) {
		goManager.ships[dmg.targetId].takeDmg(dmg.damage, dmg.damageType);
	}
}

void ClientGame::onConnectionComplete() {
	goManager.createShip(client.myUser());
	goManager.ships.at(client.myId).localPlayer = true;
}

void ClientGame::spawnShip(sf::Uint8 clientId) {
	Ship& ship = goManager.ships.at(clientId);


	ship.respawn();

	// Reset all transforms
	PhysicsTransformable& ppTrans = goManager.previousPTransformsState.at(ship.pTransId);
	ppTrans.setPosition(level.spawnPoints[client.users.at(clientId).teamId]);
	ppTrans.setRotation(0);
	ppTrans.setToRest();
	
	if(ship.owner->clientId == client.myId) {
		ppTrans.gravity = false; // Prevents dropping after spawn before user has pressed gas
		master->gui.updateSpawnTimeLabel(false, -1.0F);
	}

	goManager.currentPTransformsState.at(ship.pTransId) = ppTrans;
}

void ClientGame::onSpawnScheduled(sf::Uint8 clientId, float timeLeft) {
	if (timeLeft > 0.0F) {
		spawnTimers[clientId].start(timeLeft);
	}
	else {
		spawnShip(clientId);
	}
}

void ClientGame::onShipInit(ShipInitMessage& shipInitMsg) {
	Ship& ship = goManager.ships.at(shipInitMsg.clientId);
	ship.respawn();
	ship.takeDmg(ship.health - shipInitMsg.health, Damageable::DMG_SNEAKY);
}

void ClientGame::onShipDeath(Ship * ship) {
	if(ship->owner->clientId == client.myId) {
		master->gui.updateSpawnTimeLabel(true, -1.0F);
		goManager.currentPTransformsState[ship->pTransId].gravity = false;
	}
}

void ClientGame::render(sf::RenderWindow& window, float dt) {
	window.clear(sf::Color(50, 50, 50));

	level.draw(window);

	goManager.drawAll(window);

	if (client.connectionDone && goManager.ships.count(client.myId) == 1) {
		sf::View view = window.getView();
		view.setCenter(goManager.ships[client.myId].getPosition());
		window.setView(view);
	}

	master->gui.lastPing = client.lastPing;
	master->gui.draw(dt);

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
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == master->settings.inGameMenuKey) {
					master->gui.toggleEscMenu();
				}
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

		render(master->window, frameTime);
	}
}

void ClientGame::fixedUpdate(float dt) {
	client.update();
	
	if (client.connectionDone) {

		goManager.previousPTransformsState = goManager.currentPTransformsState;

		handleSpawnTimers(dt);

		// Input
		Input input;
		InputResponse inputRes;
		if (master->window.hasFocus()) {
			input = processInput();
			inputRes = applyInput(input, goManager.ships.at(client.myId), dt);
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

		collisionDetectAll(client.teams);
		
		goManager.deleteGarbage();

		//Send my state to server
		ShipState ss = goManager.getShipState(client.myId);
		ss.throttle = input.moveForward;
		if (inputRes.bulletId != -1) {
			ss.shoot = true;
			ss.bulletId = sf::Uint16(inputRes.bulletId);
		}
		client.sendShipUpdate(ss);
	}
}

void ClientGame::applyServerStates(ServerShipStates& sss) {
	for (auto& pair : sss.states) {

		ShipState& shipState = pair.second;
		sf::Uint8 clientId = pair.first;

		if (client.users.count(clientId) == 1) {

			Ship& ship = goManager.ships.at(clientId);

			if (clientId != client.myId && ship.isDead() == false) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));

				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
				}

				if (shipState.throttle)
					ship.throttle = true;
				else
					ship.throttle = false;
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
	if (sf::Keyboard::isKeyPressed(master->settings.moveForwardKey))
		input.moveForward = true;
	if (sf::Keyboard::isKeyPressed(master->settings.turnLeftKey))
		input.turnLeft = true;
	if (sf::Keyboard::isKeyPressed(master->settings.turnRightKey))
		input.turnRight = true;
	if (sf::Keyboard::isKeyPressed(master->settings.shootKey))
		input.shooting = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityForwardKey))
		input.abilityForward = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityLeftKey))
		input.abilityLeft = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityBackwardKey))
		input.abilityBackward = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityRightKey))
		input.abilityRight = true;
	return input;
}

InputResponse ClientGame::applyInput(Input input, Ship& ship, float dt) {
	InputResponse res; // bulletId automatically -1

	if (ship.isDead() && input.any()) {
		if (spawnRequestTimer.getElapsedTime().asSeconds() > spawnRequestBlockDuration) {
			client.requestSpawn();
			spawnRequestTimer.restart();
		}
		return res;
	}
	
	PhysicsTransformable& currTrans = goManager.currentPTransformsState[ship.pTransId];

	sf::Vector2f tempForceOnSelf;

	if (input.moveForward) {
		tempForceOnSelf += currTrans.getRotationVector() * 30.0F;
		improveHandling(ship);
#ifndef _DEBUG
		currTrans.gravity = true; // User has most likely left spawn, can apply gravity
#endif // !_DEBUG
		ship.throttle = true;
	}
	else {
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

	if (input.abilityLeft) {
		tempForceOnSelf += sf::Vector2f(-10, 0) * dt;
		console::dlog("abilityLeft");
	}
	if (input.abilityRight) {
		tempForceOnSelf += sf::Vector2f(10, 0) * dt;
		console::dlog("abilityRight");
	}

	if (input.shooting) {
		res.bulletId =  ship.weapon->shoot(); // shoots only if firerate allows
	}
	currTrans.forceOnSelf = tempForceOnSelf;
	return res;
}

void ClientGame::onReceiveKillDetails(KillDetails & killDetails) {
	std::string killerString = "";
	sf::Color killerColor = sf::Color::White;
	for(std::size_t i = 0; i < killDetails.contributors.size(); i++) {
		if(client.users.count(killDetails.contributors[i].clientId) == 1) {
			if(i == 0) {
				killerString += client.users[killDetails.contributors[i].clientId].username;
				killerColor = client.teams[client.users[killDetails.contributors[i].clientId].teamId].getColor();
				if(killDetails.contributors[i].clientId == client.myId) {
					master->gui.showPointFeedMessage("Elimination +100");
				}
			}
			else {
				if(i == 1)
					killerString += " + " + client.users[killDetails.contributors[i].clientId].username;
				else if(i == 2) {
					killerString += " + ...";
					break;
				}
				if(killDetails.contributors[i].clientId == client.myId) {
					master->gui.showPointFeedMessage("Assist +" + std::to_string(killDetails.contributors[i].dmg));
				}
			}
		}
	}
	std::string killedString = "";
	sf::Color killedColor = sf::Color::White;
	if(client.users.count(killDetails.clientKilled) == 1) {
		killedString = client.users[killDetails.clientKilled].username;
		killedColor = client.teams[client.users[killDetails.clientKilled].teamId].getColor();
	}
	if(goManager.ships.count(killDetails.clientKilled) == 1) {
		goManager.ships[killDetails.clientKilled].takeDmg(goManager.ships[killDetails.clientKilled].health, Damageable::DMG_SNEAKY);
	}

	master->gui.showKillFeedMessage(killerString, " killed ", killedString, killerColor, sf::Color::White, killedColor);
	scoreBoard.addKillDetails(killDetails);
}

// used for things the bullets and ships can't access themselves
void ClientGame::onBulletCollision(Bullet& bullet, Ship& targetShip) {
}
void ClientGame::onShipCollision(Ship& ship1, Ship& ship2) {
}

void ClientGame::handleSpawnTimers(float dt) {
	for (auto it = spawnTimers.begin(); it != spawnTimers.end();) {
		if (it->first == client.myId)
			master->gui.updateSpawnTimeLabel(true, it->second.getTimeRemaining());

		if (it->second.getTimeRemaining() <= 0.0F) {
			spawnShip(it->first);
			if (it->first == client.myId)
				master->gui.updateSpawnTimeLabel(false, it->second.getTimeRemaining());
			it = spawnTimers.erase(it);
		}
		else {
			it++;
		}
	}
}
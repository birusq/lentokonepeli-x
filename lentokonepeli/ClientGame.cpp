#include "ClientGame.h" 
#include <iostream>
#include "Raknet\RakWString.h"
#include <Thor/Vectors.hpp>
#include "User.h"
#include "Weapon.h"
#include "Master.h"
#include "Game.h"

ClientGame::ClientGame(RakNet::SystemAddress hostAddress_) : hostAddress{ hostAddress_ } {

	client.init(this);

	if (master->settings.username.getValue() == "*") {
		client.start(hostAddress, master->settings.tempUsername.c_str());
	}
	else {
		client.start(hostAddress, master->settings.username.getValue().c_str());
	}

	sf::View view = master->window.getView();
	view.setCenter(level.center);
	master->window.setView(view);

	scores.init(&client);

	guiScale = (float)master->settings.guiScalePercent / 100.0F;

	float minimapSizeRatio = min(340.0F * guiScale / level.width, 340.0F * guiScale / level.height);
	sf::Vector2f minimapPixelSize = sf::Vector2f(level.width * minimapSizeRatio, level.height * minimapSizeRatio);
	minimapSizeScreenFactor = sf::Vector2f(minimapPixelSize.x / (float)master->window.getSize().x, minimapPixelSize.y / (float)master->window.getSize().y);
}

void ClientGame::onOtherUserConnect(User& user) {
	goManager.createShip(user, user.teamId);
}

void ClientGame::beforeOtherUserDisconnect(User& user) {
	scores.deleteUser(user);
	goManager.removeShip(user.clientId);
}

void ClientGame::onTeamJoin(sf::Uint8 clientId, Team::Id newTeam) {
	std::string message;
	if (clientId == client.getMyId()) {
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

	scores.addUser(client.users.at(clientId));
}

void ClientGame::onDamage(DamageMessage& dmg) {
	if(goManager.ships.count(dmg.targetId) == 1 && goManager.ships[dmg.targetId].isDead() == false) {
		if(dmg.damageType == Damageable::DMG_SNEAKY || dmg.damageType == Damageable::DMG_UNIDENTIFIED) {
			goManager.ships[dmg.targetId].takeDmg(dmg.damage, dmg.damageType);
		}
		else if(goManager.ships.count(dmg.dealerId) == 1) {
			goManager.ships[dmg.targetId].takeDmg(dmg.damage, dmg.damageType, dmg.dealerId);
		}
	}
}

void ClientGame::onConnectionComplete() {
	goManager.createShip(*client.getMyUser());
	goManager.ships.at(client.getMyId()).localPlayer = true;
	master->gui.showPanel("chooseTeam");
}

void ClientGame::spawnShip(sf::Uint8 clientId) {
	Ship& ship = goManager.ships.at(clientId);

	if(client.isMyId(ship.owner->clientId)) {
		goManager.currentPTransformsState.at(ship.pTransId).gravity = false; // Prevents dropping after spawn before user has pressed gas
		master->gui.updateSpawnTimeLabel(false, -1.0F);
	}
	resetShipTransform(ship);
	ship.respawn();
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
	if(shipInitMsg.dead == false) {
		ship.respawn();
		ship.takeDmg(ship.health - shipInitMsg.health, Damageable::DMG_SNEAKY);
	}
}

void ClientGame::onShipDeath(Ship * ship) {
	if(client.isMyId(ship->owner->clientId)) {
		master->gui.updateSpawnTimeLabel(true, -1.0F);
	}

	resetShipTransform(*ship);
}

void ClientGame::render(sf::RenderWindow& window, float dt) {
	window.clear(sf::Color(50, 50, 50));

	level.draw(window, false);

	goManager.drawAll(window, false);

	if(master->gui.hidden == false) {
		drawMinimap(window);
	}

	master->gui.lastPing = client.lastPing;
	master->gui.draw(dt);

	window.display();
}

void ClientGame::drawMinimap(sf::RenderWindow & window) {
	sf::View oldView = window.getView();

	sf::View minimapView = sf::View();
	minimapView.setSize(level.width, level.height);
	minimapView.setViewport(sf::FloatRect(1.0F - minimapSizeScreenFactor.x, 1.0F - minimapSizeScreenFactor.y, minimapSizeScreenFactor.x, minimapSizeScreenFactor.y));
	minimapView.setCenter(level.center);

	window.setView(minimapView);

	level.draw(window, true);
	goManager.drawAll(window, true);

	window.setView(oldView);
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
			else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
				handleKeyEvents(event);
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
			inputRes = applyInput(input, goManager.ships.at(client.getMyId()), dt);
		}

		// Update my game with server states
		if (client.serverStateJitterBuffer.size() > 0) {
			if(client.serverStateJitterBuffer.back().second != nullptr) {
				applyServerStates(*client.serverStateJitterBuffer.back().second);
				client.serverStateJitterBuffer.pop_back();
			}
		}

		// Integration
		for (auto& pair : goManager.currentPTransformsState) {
			integrate(pair.second, dt);
		}

		updateAllHitboxPositions();
		collisionDetectAll(client.teams);
		
		goManager.deleteGarbage();

		//Send my state to server
		ShipState ss = goManager.getShipState(client.getMyId());

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

			if (clientId != client.getMyId() && ship.isDead() == false) {
				shipState.applyToPTrans(goManager.currentPTransformsState.at(ship.pTransId));

				if (shipState.shoot) {
					ship.weapon->shoot(shipState.bulletId, false);
					master->soundPlayer.playSound(shipState.position, "shoot");
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

	// Camera control
	if(client.connectionDone && goManager.ships.count(client.getMyId()) == 1 && goManager.ships.at(client.getMyId()).isDead() == false) {
		sf::View view = master->window.getView();
		
		// Set target to ahead of plane nose to improve visibility
		sf::Vector2f target = goManager.ships[client.getMyId()].getPosition() + goManager.ships[client.getMyId()].getRotationVector() * 20.0F;
		
		// Clamp to level borders, disabled for now because this way the ship won't go behind minimap
		/*target = sf::Vector2f(
			std::clamp(target.x, view.getSize().x / 2.0F, level.width - view.getSize().x / 2.0F),
			std::clamp(target.y, view.getSize().y / 2.0F, level.height - view.getSize().y / 2.0F));*/

		sf::Vector2f pos = view.getCenter() * (1.0F - frameTime * 2.5F) + target * frameTime * 2.5F;
		
		view.setCenter(pos);
		master->window.setView(view);
	}


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
	if (sf::Keyboard::isKeyPressed(master->settings.precisionTurnKey))
		input.precisionTurn = true;


/*
	if (sf::Keyboard::isKeyPressed(master->settings.abilityForwardKey))
		input.abilityForward = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityLeftKey))
		input.abilityLeft = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityBackwardKey))
		input.abilityBackward = true;
	if (sf::Keyboard::isKeyPressed(master->settings.abilityRightKey))
		input.abilityRight = true;
*/
	return input;
}

InputResponse ClientGame::applyInput(Input input, Ship& ship, float dt) {
	InputResponse res; // bulletId automatically -1

	if (ship.isDead() && input.shooting) {
		if (spawnRequestTimer.getElapsedTime().asSeconds() > spawnRequestBlockDuration) {
			client.requestSpawn();
			spawnRequestTimer.restart();
		}
		return res;
	}
	
	PhysicsTransformable& currTrans = goManager.currentPTransformsState[ship.pTransId];

	sf::Vector2f tempForceOnSelf;

	if (input.moveForward) {
		// The faster the ship, the less force applied
		// (allows for quick acceleration and low enough max velocity without applying massive drag)
		// (high drag feels bad cause it slows down gravity effects)
		tempForceOnSelf += currTrans.getRotationVector() * ship.maxThrottleForce * (1.0F - thor::length(ship.velocity)/ship.maxVelocity); 
		improveHandling(ship);
//#ifndef _DEBUG
		currTrans.gravity = true; // User has most likely left spawn, can apply gravity
//#endif // !_DEBUG
		ship.throttle = true;
	}
	else {
		ship.throttle = false;
	}


	float turnSpeedModif = 1.0F;
	if(ship.throttle)
		turnSpeedModif = 0.5F;


	if ((input.turnLeft && input.turnRight) || (!input.turnLeft && !input.turnRight)) {
		// Smoothly return rotation speed back to zero
		if (currTrans.angularVelocity >= 1.0F) {
			currTrans.angularVelocity -= ship.turnSpeed / ship.turnSmoothingFrames * turnSpeedModif;
		}
		else if (currTrans.angularVelocity <= -1.0F){
			currTrans.angularVelocity += ship.turnSpeed / ship.turnSmoothingFrames * turnSpeedModif;
		}
	}
	else if (input.turnLeft) {
		currTrans.angularVelocity -= ship.turnSpeed / ship.turnSmoothingFrames * turnSpeedModif;
		if (currTrans.angularVelocity < -ship.turnSpeed * turnSpeedModif) {
			currTrans.angularVelocity = -ship.turnSpeed * turnSpeedModif;
		}
	}
	else if (input.turnRight) {
		currTrans.angularVelocity += ship.turnSpeed / ship.turnSmoothingFrames * turnSpeedModif;
		if (currTrans.angularVelocity > ship.turnSpeed * turnSpeedModif) {
			currTrans.angularVelocity = ship.turnSpeed * turnSpeedModif;
		}
	}
	/*
	if (input.abilityLeft) {
		tempForceOnSelf += sf::Vector2f(-10, 0) * dt;
		console::dlog("abilityLeft");
	}
	if (input.abilityRight) {
		tempForceOnSelf += sf::Vector2f(10, 0) * dt;
		console::dlog("abilityRight");
	}
	*/

	if (input.shooting) {
		res.bulletId =  ship.weapon->shoot(); // shoots only if firerate allows
		if(res.bulletId != -1) {
			master->soundPlayer.playSound(ship.getPosition(), "shoot");
		}
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
				if(killDetails.contributors[i].clientId == client.getMyId()) {
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
				if(killDetails.contributors[i].clientId == client.getMyId()) {
					master->gui.showPointFeedMessage("Assist +" + std::to_string(killDetails.contributors[i].dmg));
				}
			}
		}
	}

	std::string killedString = "";
	sf::Color killedColor = sf::Color::White;
	if(client.isConnected(killDetails.clientKilled)) {
		killedString = client.users[killDetails.clientKilled].username;
		killedColor = client.teams[client.users[killDetails.clientKilled].teamId].getColor();
	}

	std::string verb = " killed ";

	if(killDetails.contributors.size() == 0) {
		killerString = client.users[killDetails.clientKilled].username;
		killerColor = client.teams[client.users[killDetails.clientKilled].teamId].getColor();
		verb = " committed suicide";
		killedString = "";
	}

	master->gui.showKillFeedMessage(killerString, verb , killedString, killerColor, sf::Color::White, killedColor);
	scores.addKillDetails(killDetails);
}

// used for things the bullets and ships can't access themselves
void ClientGame::onBulletCollision(Bullet& bullet, Ship& targetShip) {}

void ClientGame::onShipToShipCollision(Ship& ship1, Ship& ship2) {}

void ClientGame::onShipToGroundCollision(Ship& ship) {}

void ClientGame::onQuit() {}

void ClientGame::handleSpawnTimers(float dt) {
	for (auto it = spawnTimers.begin(); it != spawnTimers.end();) {
		if (it->first == client.getMyId())
			master->gui.updateSpawnTimeLabel(true, it->second.getTimeRemaining());

		if (it->second.getTimeRemaining() <= 0.0F) {
			spawnShip(it->first);
			if (it->first == client.getMyId())
				master->gui.updateSpawnTimeLabel(false, it->second.getTimeRemaining());
			it = spawnTimers.erase(it);
		}
		else {
			it++;
		}
	}
}

void ClientGame::handleKeyEvents(sf::Event& event) {
	if(event.type == sf::Event::KeyPressed) {
		if(event.key.code == master->settings.inGameMenuKey) {
			master->gui.togglePanel("inGameMenu");
		}
		if(event.key.code == master->settings.scoreBoardKey) {
			master->gui.showScoreboard();
		}
		if(event.key.code == master->settings.toggleGUIKey) {
			master->gui.hidden = !master->gui.hidden;
		}
		if(event.key.code == sf::Keyboard::O) {
			master->settings.guiScalePercent.setValue(master->settings.guiScalePercent - 10);
			master->gui.updateScale();
			upateSFMLGUIScale();
		}
		if(event.key.code == sf::Keyboard::P) {
			master->settings.guiScalePercent.setValue(master->settings.guiScalePercent + 10);
			master->gui.updateScale();
			upateSFMLGUIScale();
		}
	}
	if(event.type == sf::Event::KeyReleased) {
		if(event.key.code == master->settings.scoreBoardKey) {
			master->gui.hideScoreboard();
		}
	}
}

void ClientGame::upateSFMLGUIScale() {
	float newGUIScale = (float)master->settings.guiScalePercent / 100.0F;
	float multiplier = newGUIScale / guiScale;

	minimapSizeScreenFactor *= multiplier;

	guiScale = newGUIScale;
}

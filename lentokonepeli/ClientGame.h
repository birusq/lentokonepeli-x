#pragma once

#include "Game.h"
#include "Client.h"
#include "TestLevel.h"
#include <optional>

class ClientGame : public Game {
public:
	ClientGame(std::string hostIp_);

	void loop();

	Client client;

	// Fired when user data (clientId etc.) is received from server
	void onConnectionComplete();

	void onOtherUserConnect(User* const user);

	void onOtherUserDisconnect(sf::Uint8 clientId);

	void onTeamJoin(sf::Uint8 clientId, TeamId newTeam);

	void onBulletHit(sf::Uint8 shooterId, sf::Uint16 bulletId, sf::Uint8 targetId, sf::Uint16 damage);

	void respawnMyShip();

private:

	void applyServerStates(ServerShipStates& sss);

	std::string hostIp;

	void render(sf::RenderWindow&, GOManager&);

	Input processInput();

	int applyInput(Input input, Ship& ship, float dt);

	void fixedUpdate(float dt);

	void update(float frameTime, float alpha);

	sf::Clock inputDisabledTimer;
	float inputDisabledTime = 0.5F;
};
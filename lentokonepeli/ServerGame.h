#pragma once
#include "Game.h"
#include "Server.h"


class ServerGame : public Game {
public:
	ServerGame(Master* master_);

	Server server;

	void loop();

	void onUserConnect(User* const user);

	void onUserDisconnect(sf::Uint8 clientId);

	void onClientJoinTeam(sf::Uint8 clientId, TeamId newTeam);

private:

	void render(sf::RenderWindow&);

	void fixedUpdate(float dt);

	void applyServerStates(ServerShipStates& sss);

	void updateServerStates(ServerShipStates& sss);

	void collisionDetectAll();
};
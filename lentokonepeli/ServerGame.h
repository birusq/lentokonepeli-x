#pragma once
#include "Game.h"
#include "Server.h"


class ServerGame : public Game {
public:
	ServerGame();

	Server server;

	void loop();

	void onUserConnect(User* const user);

	void onUserDisconnect(sf::Uint8 clientId);

	void onClientJoinTeam(sf::Uint8 clientId, Team::Id newTeam);
	
	void onSpawnRequest(sf::Uint8 clientId);

	void fillShipInit(User& user, ShipInitMessage& shipInitMsg);

	void onShipDeath(Ship* ship) override;

protected:
	void onBulletCollision(Bullet& bullet, Ship& targetShip) override;
	void onShipCollision(Ship& s1, Ship& s2) override;
private:

	void render(sf::RenderWindow&, float dt);

	void fixedUpdate(float dt);

	void applyClientShipStates(ServerShipStates& sss, float dt);

	void updateServerStates(ServerShipStates& sss);

	void spawnShip(sf::Uint8 clientId) override;
};
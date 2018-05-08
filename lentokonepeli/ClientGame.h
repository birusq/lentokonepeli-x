#pragma once

#include "Master.h"
#include "Game.h"
#include "Client.h"
#include "TestLevel.h"

class ClientGame : public Game {
public:
	ClientGame(Master* master_, std::string hostIp_);

	void loop();

	Client client;

	// Fired when user data (clientId etc.) is received from server
	void onConnectionComplete();

	// Fired when spawning allowed from server side after dying
	void onSpawnAllowed();

	void onOtherUserConnect(User* const user);

	void onOtherUserDisconnect(uchar clientId);

private:

	void applyServerStates(ServerShipStates& sss);

	std::string hostIp;

	void render(sf::RenderWindow&, GOManager&);

	Input processInput();
	void applyInput(Input input, PhysicsTransformable& controlTarget, float dt);

	void fixedUpdate(float dt);

	void update(float frameTime, float alpha);
};
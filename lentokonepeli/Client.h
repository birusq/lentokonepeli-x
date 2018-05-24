#pragma once

#include <string>
#include "Raknet\RakPeerInterface.h"
#include "Raknet\MessageIdentifiers.h"
#include <unordered_map>
#include <SFML/System.hpp>
#include "Team.h"
#include <deque>
#include "PacketHelper.h"
#include "User.h"

using namespace RakNet;

class Master;
class ClientGame;

class Client {
public:
	Client() {}
	~Client();

	void init(ClientGame* game_);

	void start(std::string hostIp, RakString username);

	void update();

	void sendShipUpdate(ShipState& shipState);

	User* const myUser() { return &users.at(myId); }
	
	std::unordered_map<sf::Uint8, User> users;
	std::unordered_map<TeamId, Team> teams;
	std::deque<ServerShipStates> serverStateJitterBuffer;
	const unsigned int jitterBufferMaxSize = 3;

	RakPeerInterface* peer = nullptr;
	int lastPing = -1;

	void requestTeamJoin(TeamId toTeam);

	void close();

	sf::Uint8 myId;
	bool connectionDone;

private:

	RakString myUsername;

	ClientGame* game;

	void processUser(Packet* packet);
	void processTeamUpdate(Packet* packet);
	void handleOtherUserDisconnect(Packet* packet);
	void processShipUpdate(Packet* packet);
	void processBulletHit(Packet* packet);
	void processShipsCollision(Packet* packet);

	RakNetGUID hostguid;
};
#pragma once

#include "Game.h"
#include "Raknet\RakPeerInterface.h"
#include "Raknet\MessageIdentifiers.h"
#include "User.h"
#include <unordered_map>
#include <vector>
#include "Team.h"

using namespace RakNet;

class ServerGame;

class Server {
public:
	~Server();

	void init(Master* master_, ServerGame* game_);

	void start(uchar maxClients);

	void close();

	void update();
		
	RakPeerInterface* peer = nullptr;
	
	std::unordered_map<uchar, User> users;
	std::unordered_map<uchar, std::deque<ShipState>> shipStateJitterBuffers;
	std::unordered_map<TeamId, Team> teams;

	void sendAllowSpawnMsg(uchar clientId);

	void broadcastShipStates(ServerShipStates& newStates);

private:
	Master* master;
	ServerGame* game;

	void changeTeam(TeamId newTeam, uchar clientId);

	void sendAllUsersUpdate(SystemAddress toAddress);
	void sendUserUpdate(User& user, SystemAddress toAddress, bool broadcast);

	void broadcastUserDisconnect(User& user);

	void handleUserUpdate(Packet* packet);
	void handleUserDisconnect();

	void handleJoinTeamReq(Packet* packet);

	void sendTeamUpdate();

	void handleShipUpdate(Packet* packet);

	int timer;
};
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

	void init(ServerGame* game_);

	void start(sf::Uint8 maxClients);

	void close();

	void update();
		
	RakPeerInterface* peer = nullptr;
	
	std::unordered_map<sf::Uint8, User> users;
	std::unordered_map<sf::Uint8, std::deque<ShipState>> shipStateJitterBuffers;
	std::unordered_map<Team::Id, Team> teams;

	void broadcastShipStates(ServerShipStates& newStates);

	void sendBulletHitShip(Bullet* bullet, Ship* targetShip);
	void sendShipsCollided(Ship* s1, Ship* s2);

	void sendShipSpawn(sf::Uint8 clientId, bool canSpawn, float timeUntilSpawn, SystemAddress toAddress = UNASSIGNED_SYSTEM_ADDRESS, bool broadcast = true);

private:
	ServerGame* game;

	void changeTeam(Team::Id newTeam, sf::Uint8 clientId);

	void sendUserUpdate(User& user, SystemAddress toAddress, bool broadcast);

	void broadcastUserDisconnect(User& user);

	void handleUserUpdate(Packet* packet);
	void handleUserDisconnect();

	void handleJoinTeamReq(Packet* packet);

	void sendTeamUpdate(sf::Uint8 oldTeam, sf::Uint8 newTeam, sf::Uint8 clientId, SystemAddress toAddress, bool broadcast);

	void handleShipUpdate(Packet* packet);

	void handleSpawnReq(Packet* packet);

	int timer;
};
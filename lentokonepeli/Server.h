#pragma once

#include "Game.h"
#include "Raknet\RakPeerInterface.h"
#include "Raknet\MessageIdentifiers.h"
#include "User.h"
#include <unordered_map>
#include <vector>
#include "Team.h"
#include "NetworkAgent.h"

using namespace RakNet;

class ServerGame;

class Server : public NetworkAgent {
public:
	~Server();

	void init(ServerGame* game_);

	void start(sf::Uint8 maxClients, unsigned short port = SERVER_DEFAULT_PORT);
	
	std::unordered_map<sf::Uint8, std::vector<std::pair<sf::Uint16, std::shared_ptr<ShipState>>>> shipStateJitterBuffers;

	void broadcastShipStates(ServerShipStates& newStates);

	void sendDamage(DamageMessage& dmg);

	void sendShipSpawn(sf::Uint8 clientId, bool canSpawn, float timeUntilSpawn, SystemAddress toAddress = UNASSIGNED_SYSTEM_ADDRESS, bool broadcast = true);

	void broadcastKillDetails(KillDetails& killDetails);

	void close() override;

	void update() override;

	bool isMyId(sf::Uint8 clientId) override { return false; }

private:
	ServerGame* game;

	void changeTeam(Team::Id newTeam, sf::Uint8 clientId);

	void broadcastUserDisconnect(User& user);

	void sendShipInit(User& user, SystemAddress toAddress);
	void sendUserUpdate(User& user, SystemAddress toAddress, bool broadcast);
	void sendTeamUpdate(sf::Uint8 oldTeam, sf::Uint8 newTeam, sf::Uint8 clientId, SystemAddress toAddress, bool broadcast);
	void sendScoresUpdate(SystemAddress toAddress, bool broadcast);

	void handleUserUpdate(Packet* packet);
	void handleUserDisconnect();
	void handleJoinTeamReq(Packet* packet);
	void handleShipUpdate(Packet* packet);
	void handleSpawnReq(Packet* packet);

	int timer;

	std::size_t jitterBufferMaxSize = 5;
	sf::Uint16 currentSeqNum = 0;
};
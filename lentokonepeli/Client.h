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
#include "NetworkAgent.h"

using namespace RakNet;

class Master;
class ClientGame;

class Client : public NetworkAgent {
public:
	Client() {}
	~Client();

	void init(ClientGame* game_);

	void start(std::string hostIp, RakString username);

	void update() override;

	void close() override;

	void sendShipUpdate(ShipState& shipState);

	User* const getMyUser() { return &users.at(myId); }

	std::deque<ServerShipStates> serverStateJitterBuffer;
	const unsigned int jitterBufferMaxSize = 3;

	int lastPing = -1;

	void requestTeamJoin(Team::Id toTeam);

	// Server responds to this with seconds until can respawn, negative if already can
	void requestSpawn();

	bool isMyId(sf::Uint8 clientId) override { return clientId == myId; }

	sf::Uint8 getMyId() { return myId; }

	bool connectionDone;

private:

	sf::Uint8 myId;

	RakString myUsername;

	ClientGame* game;

	void processUser(Packet* packet);
	void processTeamUpdate(Packet* packet);
	void handleOtherUserDisconnect(Packet* packet);
	void processShipUpdate(Packet* packet);
	void processDamage(Packet* packet);
	void processSpawnAfterTime(Packet* packet);
	void processShipInit(Packet* packet);
	void processKillDetails(Packet* packet);
	void processScoresUpdate(Packet* packet);

	RakNetGUID hostguid;
};
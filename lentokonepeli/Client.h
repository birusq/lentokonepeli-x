#pragma once


#include "Team.h"
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

	void start(SystemAddress hostIp, RakString username);

	void update() override;

	void close() override;

	void sendShipUpdate(ShipState& shipState);

	User* const getMyUser() { return &users.at(myId); }

	const unsigned int jitterBufferMaxSize = 4;
	std::vector<std::pair<sf::Uint16, std::shared_ptr<ServerShipStates>>> serverStateJitterBuffer;

	int lastPing = -1;

	void requestTeamJoin(Team::Id toTeam);

	bool teamJoinInProgress = false;

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

	sf::Uint16 currentSeqNum = 0;
};
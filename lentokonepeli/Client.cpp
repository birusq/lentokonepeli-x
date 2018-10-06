#include "Client.h"
#include "PacketHelper.h"
#include "Console.h"
#include "Master.h"
#include "ClientGame.h"
#include "User.h"
#include "Scores.h"

using namespace ph;

void Client::init(ClientGame* game_) {
	game = game_;
	master->gui.client = this;

	peer = RakPeerInterface::GetInstance();

	StartupResult res = peer->Startup(1, &SocketDescriptor(), 1);
	if (res != StartupResult::RAKNET_ALREADY_STARTED && res != StartupResult::RAKNET_STARTED) {
		console::log("Raknet could not be started, error code: " + (int)res);
	}
	else {
		console::log("Client initalized");
	}

	for (int i = 0; i < Team::TEAMS_SIZE; i++) {
		teams[(Team::Id)i] = Team((Team::Id)i);
	}
}

void Client::close() {
	connectionDone = false;
	peer->Shutdown(100);
}

Client::~Client() {
	if (peer->IsActive())
		close();
	RakPeerInterface::DestroyInstance(peer);
}

void Client::start(SystemAddress hostAddress, RakString username) {
	myUsername = username;

	ConnectionAttemptResult res = peer->Connect(hostAddress.ToString(false), hostAddress.GetPort(), 0, 0);
	if (res != ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED) {
		console::dlog("Couldn't create connection, error: " + std::to_string((int)res));
	}
	else {
		console::stream << "Trying to join address " << hostAddress.ToString(false) << " on port " << hostAddress.GetPort();
		console::logStream();
	}
	peer->SetOccasionalPing(true);

#ifdef _DEBUG
	peer->ApplyNetworkSimulator(0.05F, 30, 10);
#endif // _DEBUG
}

void Client::requestTeamJoin(Team::Id toTeam) {
	if(!teamJoinInProgress) {
		teamJoinInProgress = true;
		console::dlog("Sending team join request: " + std::to_string(toTeam));
		BitStream bitStream;
		bitStream.Write((MessageID)ID_JOIN_TEAM_REQUEST);
		bitStream.Write((unsigned char)toTeam);
		peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, hostguid, false);
	}
}

void Client::requestSpawn() {
	console::dlog("Sending spawn timer request");
	BitStream bitStream;
	bitStream.Write((MessageID)ID_SPAWN_REQUEST);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, hostguid, false);
}

void Client::update() {
	Packet* packet = peer->Receive();

	while (packet != 0) {

		MessageID packetId = getPacketIdentifier(packet);

		if (packetId == ID_CONNECTION_ATTEMPT_FAILED) {
			console::log("Connection failed, possible causes:\nincorrect ip address,\nincorrect port number\nserver isn't running,\nserver hasn't opened the port " + std::to_string(master->hostAddress.GetPort()));
		}
		else if (packetId == ID_CONNECTION_REQUEST_ACCEPTED) {
			console::log("Connection accepted");
			hostguid = packet->guid;

			BitStream bitStream;
			bitStream.Write((MessageID)ID_USER_UPDATE);
			bitStream.Write(myUsername);
			peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, hostguid, false);
		}
		else if (packetId == ID_NO_FREE_INCOMING_CONNECTIONS) {
			console::log("Connection denied, server full");
		}
		else if (packetId == ID_USER_UPDATE) {
			processUser(packet);
		}
		else if (packetId == ID_USER_DISCONNECT) {
			handleOtherUserDisconnect(packet);
		}
		else if (packetId == ID_DISCONNECTION_NOTIFICATION) {
			console::log("Disconnected, server has been closed");
		}
		else if (packetId == ID_JOIN_TEAM_FAILED_TEAM_FULL) {
			console::log("Couldn't join: team full");
			teamJoinInProgress = false;
		}
		else if (packetId == ID_TEAM_UPDATE) {
			processTeamUpdate(packet);
		}
		else if (packetId == ID_SHIP_UPDATE) {
			processShipUpdate(packet);
		}
		else if (packetId == ID_DAMAGE_DEALT) {
			processDamage(packet);
		}
		else if (packetId == ID_SPAWN_NOT_ALLOWED) {
			console::dlog("Spawn isn't allowed now");
		}
		else if (packetId == ID_SPAWN_AFTER_TIME) {
			processSpawnAfterTime(packet);
		}
		else if (packetId == ID_SHIP_INIT) {
			processShipInit(packet);
		}
		else if(packetId == ID_KILL_DETAILS) {
			processKillDetails(packet);
		}
		else if(packetId == ID_SCORES_UPDATE) {
			processScoresUpdate(packet);
		}
		else {
			if (packetId <= 134)
				console::log("Packet type not handled: " + (std::string)PacketLogger::BaseIDTOString(packetId));
			else
				console::log("Packet type not handled: " + (int)(MessageIndetifier)packetId);
		}

		peer->DeallocatePacket(packet);
		packet = peer->Receive();
	}

	lastPing = peer->GetLastPing(peer->GetSystemAddressFromIndex(0));
}

void Client::processUser(Packet* packet) {
	User user;
	unsigned char c;

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.Read(c);
	User::serialize(bitStream, user, false);

	bool newUser = false;
	if (users.count(user.clientId) == 0)
		newUser = true;

	users[user.clientId] = user;

	if (user.guid == peer->GetMyGUID()) {
		myId = user.clientId;
		connectionDone = true;
		game->onConnectionComplete();
	}
	else if (newUser) {
		// create ship for new user
		game->onOtherUserConnect(users[user.clientId]);
	}

	users[user.clientId] = user;

	teams[user.teamId].members.push_back(user.clientId);

	console::dlog("User update: " + std::to_string((unsigned int)user.clientId) + (std::string)user.username + user.guid.ToString());
}

void Client::handleOtherUserDisconnect(Packet* packet) {
	User user;
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	User::serialize(bitStream, user, false);

	console::log((std::string)user.username.C_String() + " disconnected");

	game->beforeOtherUserDisconnect(user);

	teams[user.teamId].removeClient(user.clientId);

	users.erase(user.clientId);
}

void Client::processTeamUpdate(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	TeamChange tc;
	tc.serialize(bitStream, false);

	Team::Id oldTeam = (Team::Id)tc.oldTeamId;
	Team::Id newTeam = (Team::Id)tc.newTeamId;
	sf::Uint8 clientId = tc.clientId;

	teams[oldTeam].removeClient(clientId);

	teams[newTeam].members.push_back(clientId);
	users.at(clientId).teamId = newTeam;

	game->onTeamJoin(clientId, newTeam);
}

void Client::processShipUpdate(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	sf::Uint16 seqNum;
	bitStream.Read(seqNum);

	auto newServerState = std::make_shared<ServerShipStates>();
	newServerState->serialize(bitStream, false);

	//console::dlog("Original seq: " + std::to_string((int)seqNum));

	// Set state to the right position based on seqnumber, preserving order
	if(serverStateJitterBuffer.size() == 0) {
		serverStateJitterBuffer.push_back(std::make_pair(seqNum, newServerState));
	}
	else {
		for(std::size_t i = 0; i < serverStateJitterBuffer.size(); i++) {
			if(ph::seqGreaterThan(seqNum, serverStateJitterBuffer[i].first)) {
				serverStateJitterBuffer.insert(serverStateJitterBuffer.begin() + i, std::make_pair(seqNum, newServerState));
				break;
			}
			else if(seqNum == serverStateJitterBuffer[i].first) {
				serverStateJitterBuffer[i] = std::make_pair(seqNum, newServerState);
				break;
			}
		}
	}

	seqNum = serverStateJitterBuffer[0].first;

	// fill gaps with nulls to prevent jumping
	for(std::size_t i = 1; i < serverStateJitterBuffer.size(); i++) {
		if(ph::seqGreaterThan(seqNum, serverStateJitterBuffer[i].first + 1)) {
			serverStateJitterBuffer.insert(serverStateJitterBuffer.begin() + i, std::make_pair(seqNum - 1, nullptr));
		}
		seqNum--;
	}

	// Trim out too old messages
	while(serverStateJitterBuffer.size() > jitterBufferMaxSize) {
		serverStateJitterBuffer.pop_back();
	}

	// Debugging
	/*for(std::size_t i = 0; i < serverStateJitterBuffer.size(); i++) {
		std::string s = "";
		if(serverStateJitterBuffer[i].second == nullptr) {
			s = " = null";
		}
		console::dlog("Seq fixed: " + std::to_string((int)serverStateJitterBuffer[i].first) + s);
	}*/

	//console::dlog("Ship update received");
}

void Client::processDamage(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	
	DamageMessage dmg;
	dmg.serialize(bitStream, false);

	game->onDamage(dmg);
}

void Client::processSpawnAfterTime(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	
	sf::Uint8 clientId;
	bitStream.Read(clientId);

	float timeUntilSpawn;
	bitStream.Read(timeUntilSpawn);

	if (peer->GetLastPing(packet->guid) > 0) {
		timeUntilSpawn -= (float)peer->GetLastPing(packet->guid) / 1000.0F;
	}

	console::stream << "Spawn response received, will spawn in " << timeUntilSpawn << " seconds";
	console::dlogStream();
	game->onSpawnScheduled(clientId, timeUntilSpawn);
}

void Client::processShipInit(Packet * packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	ShipInitMessage shipInitMsg;
	shipInitMsg.serialize(bitStream, false);

	console::stream << "Ship initalized for client " << (int)shipInitMsg.clientId;
	console::dlogStream();
	game->onShipInit(shipInitMsg);
}

void Client::processKillDetails(Packet * packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	KillDetails killDetails;
	killDetails.serialize(bitStream, false);

	console::stream << "Kill details received for client " << (int)killDetails.clientKilled;
	console::dlogStream();

	game->onReceiveKillDetails(killDetails);
}

void Client::processScoresUpdate(Packet * packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	game->scores.serialize(bitStream, false);

	console::stream << "Scores update received";
	console::dlogStream();
}

void Client::sendShipUpdate(ShipState& shipState) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_SHIP_UPDATE);
	
	bitStream.Write(currentSeqNum);
	currentSeqNum++;

	shipState.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, UNRELIABLE, 0, hostguid, false);
}
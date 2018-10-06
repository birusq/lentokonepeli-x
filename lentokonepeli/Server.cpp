#include "Server.h"
#include "PacketHelper.h"
#include "Console.h"
#include "ServerGame.h"
#include "Bullet.h"
#include "Master.h"
#include "Scores.h"


using namespace ph;

void Server::init(ServerGame* game_) {
	game = game_;
	master->gui.server = this;

	peer = RakPeerInterface::GetInstance();
	peer->SetUnreliableTimeout(5000);
}

void Server::start(sf::Uint8 maxClients, unsigned short port) {
	StartupResult res = peer->Startup(maxClients, &SocketDescriptor(port, 0), 1);
	if(res == StartupResult::SOCKET_PORT_ALREADY_IN_USE) {
		unsigned short newPort = std::clamp((unsigned short)(port + 1), (unsigned short)10000, UINT16_MAX);
		console::stream << "Couldn't start server, port " << port << " is already in use, trying port " << newPort;
		console::logStream();
		start(maxClients, newPort);
		return;
	}
	if (res != StartupResult::RAKNET_ALREADY_STARTED && res != StartupResult::RAKNET_STARTED) {
		console::log("Raknet could not be started, error code: " + res);
	}
	else {
		console::stream << "Server started on port " << port << ", max clients: " << (int)maxClients;
		console::logStream();
	}

	peer->SetMaximumIncomingConnections(maxClients);

	for (int i = 0; i < Team::TEAMS_SIZE; i++) {
		teams[(Team::Id)i] = Team((Team::Id)i);
	}

#ifdef _DEBUG
	peer->ApplyNetworkSimulator(0.05F, 30, 10);
#endif // _DEBUG
}

void Server::changeTeam(Team::Id newTeam, sf::Uint8 clientId) {
	Team::Id oldTeam = users.at(clientId).teamId;

	teams[oldTeam].removeClient(clientId);

	teams[newTeam].members.push_back(clientId);
	users.at(clientId).teamId = newTeam;

	game->onClientJoinTeam(clientId, oldTeam, newTeam);

	sendTeamUpdate(oldTeam, newTeam, clientId, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Server::update() {
	Packet* packet = peer->Receive();
	while (packet != 0) {

		MessageID packetId = ph::getPacketIdentifier(packet);

		if (packetId == ID_NEW_INCOMING_CONNECTION) {
			console::log("New connection " + std::to_string(peer->GetIndexFromSystemAddress(packet->systemAddress)));
		}
		else if (packetId == ID_DISCONNECTION_NOTIFICATION) {
			handleUserDisconnect();
		}
		else if (packetId == ID_CONNECTION_LOST) {
			handleUserDisconnect();
		}
		else if (packetId == ID_USER_UPDATE) { // Client sends this just after connecting
			handleUserUpdate(packet);
		}
		else if (packetId == ID_SHIP_UPDATE) {
			handleShipUpdate(packet);
		}
		else if (packetId == ID_JOIN_TEAM_REQUEST) {
			handleJoinTeamReq(packet);
		}
		else if (packetId == ID_SPAWN_REQUEST) {
			handleSpawnReq(packet);
		}
		else {
			if (packetId <= 134)
				console::log("Packet type not handled: " + (std::string)PacketLogger::BaseIDTOString(packetId));
			else
				console::log("Packet type not handled: " + (std::string)msgIDToString((MessageIndetifier)packetId));
		}

		peer->DeallocatePacket(packet);
		packet = peer->Receive();
	}
}

void Server::close() {
	peer->Shutdown(100);
}

Server::~Server() {
	if (peer->IsActive())
		close();
	RakPeerInterface::DestroyInstance(peer);
}

void Server::handleUserUpdate(Packet* packet) {
	User user;
	user.clientId = peer->GetIndexFromSystemAddress(packet->systemAddress);
	user.guid = packet->guid;

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	bitStream.Read(user.username);

	bool newUser = false;
	if (users.count(user.clientId) == 0)
		newUser = true;

	users[user.clientId] = user;

	if (newUser) {
		game->onUserConnect(users[user.clientId]);

		while (shipStateJitterBuffers[user.clientId].size() < jitterBufferMaxSize) {
			shipStateJitterBuffers[user.clientId].push_back(std::make_pair(0, nullptr));
		}

		// send data of all users to new connected user
		for (auto& pair : users) {
			if (pair.second.clientId != user.clientId) {
				sendUserUpdate(pair.second, packet->systemAddress, false);
				sendShipInit(pair.second, packet->systemAddress);
				sendScoresUpdate(packet->systemAddress, false);
			}
		}
	}

	sendUserUpdate(user, UNASSIGNED_SYSTEM_ADDRESS , true); // send data of this user to all others

	//console::log("User update: " + std::to_string(user.clientId) + (std::string)user.username.C_String() + user.guid.ToString());
}

void Server::sendUserUpdate(User& user, SystemAddress toAddress, bool broadcast) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_USER_UPDATE);
	User::serialize(bitStream, user, true);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, toAddress, broadcast);
}

void Server::handleUserDisconnect() {
	for (auto& it = users.begin(); it != users.end();) {
		ConnectionState cs = peer->GetConnectionState(it->second.guid);
		if (cs == IS_DISCONNECTED || cs == IS_DISCONNECTING || cs == IS_NOT_CONNECTED || cs == IS_SILENTLY_DISCONNECTING) {
			broadcastUserDisconnect(it->second);
			console::log("User disconnected: " + (std::string)it->second.username.C_String());
			shipStateJitterBuffers.erase(it->second.clientId);

			game->beforeUserDisconnect(it->second);

			teams[it->second.teamId].removeClient(it->second.clientId);
			it = users.erase(it);
		}
		else it++;
	}
}

void Server::broadcastUserDisconnect(User& user) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_USER_DISCONNECT);
	User::serialize(bitStream, user, true);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 1, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Server::sendShipInit(User& user, SystemAddress toAddress) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_SHIP_INIT);
	
	ShipInitMessage shipInitMsg;
	game->fillShipInit(user, shipInitMsg);

	shipInitMsg.serialize(bitStream, true);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 1, toAddress, false);
}

void Server::handleJoinTeamReq(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	Team::Id toTeam;
	bitStream.Read(toTeam);

	// possibly in future check more restrictions
	sf::Uint32 maxPlayers = peer->GetMaximumIncomingConnections();
	if ((float)teams[toTeam].members.size() >= ((float)maxPlayers / 2.0F)) {
		BitStream bs;
		bs.Write((MessageID)ID_JOIN_TEAM_FAILED_TEAM_FULL);
		peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 2, packet->systemAddress, false);
		return;
	}

	changeTeam(toTeam, peer->GetIndexFromSystemAddress(packet->systemAddress));
}

void Server::sendTeamUpdate(sf::Uint8 oldTeam, sf::Uint8 newTeam, sf::Uint8 clientId, SystemAddress toAddress, bool broadcast) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_TEAM_UPDATE);

	TeamChange tc = TeamChange(oldTeam, newTeam, clientId);
	tc.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 4, toAddress, broadcast);

	console::dlog("Team update sent");
}

void Server::sendScoresUpdate(SystemAddress toAddress, bool broadcast) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_SCORES_UPDATE);

	game->scores.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 4, toAddress, broadcast);

	console::dlog("Score update sent");
}

void Server::handleShipUpdate(Packet* packet) {
	sf::Uint8 clientId = peer->GetIndexFromSystemAddress(packet->systemAddress);

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	sf::Uint16 seqNum;
	bitStream.Read(seqNum);

	auto newShipState = std::make_shared<ShipState>();
	newShipState->serialize(bitStream, false);
	
	//console::dlog("Original seq: " + std::to_string((int)seqNum));

	// Set state to the right position based on seqnumber, preserving order
	if(shipStateJitterBuffers[clientId].size() == 0) {
		shipStateJitterBuffers[clientId].push_back(std::make_pair(seqNum, newShipState));
	}
	else {
		for(std::size_t i = 0; i < shipStateJitterBuffers[clientId].size(); i++) {
			if(ph::seqGreaterThan(seqNum, shipStateJitterBuffers[clientId][i].first)) {
				shipStateJitterBuffers[clientId].insert(shipStateJitterBuffers[clientId].begin() + i, std::make_pair(seqNum, newShipState));
				break;
			}
			else if (seqNum == shipStateJitterBuffers[clientId][i].first) {
				shipStateJitterBuffers[clientId][i] = std::make_pair(seqNum, newShipState);
				break;
			}
		}
	}

	seqNum = shipStateJitterBuffers[clientId][0].first;

	// fill gaps with nulls to prevent jumping
	for(std::size_t i = 1; i < shipStateJitterBuffers[clientId].size(); i++) {
		if(ph::seqGreaterThan(seqNum, shipStateJitterBuffers[clientId][i].first + 1)) {
			shipStateJitterBuffers[clientId].insert(shipStateJitterBuffers[clientId].begin() + i, std::make_pair(seqNum - 1, nullptr));
		}
		seqNum--;
	}

	// Trim out too old messages
	while (shipStateJitterBuffers[clientId].size() > jitterBufferMaxSize) {
		shipStateJitterBuffers[clientId].pop_back();
	}
	
	// Debugging
	/*for(std::size_t i = 0; i < shipStateJitterBuffers[clientId].size(); i++) {
		std::string s = "";
		if(shipStateJitterBuffers[clientId][i].second == nullptr) {
			s = " = null";
		}
		console::dlog("Seq fixed: " + std::to_string((int)shipStateJitterBuffers[clientId][i].first) + s);
	}*/
	

	//console::dlog("Received ship state from " + std::to_string((int)clientId) + ", ship ypos: " + std::to_string(shipStates[clientId].position.y));
}

void Server::handleSpawnReq(Packet * packet) {
	console::stream << "Got spawn request from " << users[peer->GetIndexFromSystemAddress(packet->systemAddress)].username.C_String();
	console::dlogStream();
	game->onSpawnRequest(peer->GetIndexFromSystemAddress(packet->systemAddress));
}


void Server::broadcastShipStates(ServerShipStates& newStates) {
	if (newStates.states.size() == 0)
		return;

	BitStream bitStream;
	bitStream.Write((MessageID)ID_SHIP_UPDATE);

	bitStream.Write(currentSeqNum);
	currentSeqNum++;

	newStates.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, UNRELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	//console::dlog("Number of ships in update: " + std::to_string(newStates.states.size()));
}

void Server::sendDamage(DamageMessage& dmg) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_DAMAGE_DEALT);
	dmg.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Server::sendShipSpawn(sf::Uint8 clientId, bool canSpawn, float timeUntilSpawn, SystemAddress toAddress, bool broadcast) {
	BitStream bitStream;

	if (canSpawn == false) {
		// Only send to requester if spawn not allowed
		bitStream.Write((MessageID)ID_SPAWN_NOT_ALLOWED);
		peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, users[clientId].guid, false);

		console::stream << "Sent spawn not allowed message to " << users[clientId].username.C_String();
		console::dlogStream();
	}
	else {

		bitStream.Write((MessageID)ID_SPAWN_AFTER_TIME);
		bitStream.Write(clientId);
		bitStream.Write(timeUntilSpawn);

		peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, toAddress, broadcast);

		console::stream << "Broadcasting spawn " << users[clientId].username.C_String() << " in " << timeUntilSpawn << " seconds";
		console::dlogStream();
	}
}

void Server::broadcastKillDetails(KillDetails& killDetails) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_KILL_DETAILS);
	killDetails.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

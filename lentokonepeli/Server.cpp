#include "Server.h"
#include <iostream>
#include "PacketHelper.h"
#include "Raknet\PacketLogger.h"
#include "Raknet\BitStream.h"
#include "Console.h"
#include "ServerGame.h"

using namespace ph;


void Server::init(Master* master_, ServerGame* game_) {
	master = master_;
	game = game_;
	master->gui.server = this;

	peer = RakPeerInterface::GetInstance();
	peer->SetUnreliableTimeout(5000);
}

void Server::start(uchar maxClients) {
	StartupResult res = peer->Startup(maxClients, &SocketDescriptor(SERVER_PORT, 0), 1);
	if (res != StartupResult::RAKNET_ALREADY_STARTED && res != StartupResult::RAKNET_STARTED) {
		console::log("Raknet could not be started, error code: " + res);
	}
	else {
		console::log("Server started, max clients: " + std::to_string(maxClients));
	}

	peer->SetMaximumIncomingConnections(maxClients);

	for (int i = 0; i < TEAMS_SIZE; i++) {
		teams[(TeamId)i] = Team((TeamId)i);
	}
}

void Server::changeTeam(TeamId newTeam, uchar clientId) {
	for (uchar i = 0; i < TEAMS_SIZE; i++) {
		auto it = std::find(teams[(TeamId)i].members.begin(), teams[(TeamId)i].members.end(), clientId);
		if (it != teams[(TeamId)i].members.end()) {
			teams[(TeamId)i].members.erase(it);
			break;
		}
	}

	teams[newTeam].members.push_back(clientId);

	game->onClientJoinTeam(clientId, newTeam);
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

	changeTeam(TeamId::NO_TEAM, user.clientId);

	bool newUser = false;
	if (users.count(user.clientId) == 0)
		newUser = true;

	users[user.clientId] = user;

	sendAllUsersUpdate(packet->systemAddress); // send data of all users to connected user
	sendUserUpdate(user, packet->systemAddress, true); // send data of this user to all others

	if (newUser)
		game->onUserConnect(&user);

	console::log("User update: " + std::to_string(user.clientId) + (std::string)user.username.C_String() + user.guid.ToString());
}

void Server::sendAllUsersUpdate(SystemAddress toAddress) {
	for (auto& pair : users) {
		sendUserUpdate(pair.second, toAddress, false);
	}
}

void Server::sendUserUpdate(User& user, SystemAddress toAddress, bool broadcast) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_USER_UPDATE);
	User::serialize(bitStream, user, true);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, toAddress, broadcast);
}

void Server::handleUserDisconnect() {
	for (auto it = users.begin(); it != users.end();) {
		ConnectionState cs = peer->GetConnectionState(it->second.guid);
		if (cs == IS_DISCONNECTED || cs == IS_DISCONNECTING || cs == IS_NOT_CONNECTED || cs == IS_SILENTLY_DISCONNECTING) {
			broadcastUserDisconnect(it->second);
			console::log("User disconnected: " + (std::string)it->second.username.C_String());
			shipStateJitterBuffers.erase(it->second.clientId);
			game->onUserDisconnect(it->second.clientId);
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

void Server::handleJoinTeamReq(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	TeamId toTeam;
	bitStream.Read(toTeam);

	// possibly in future check more restrictions
	uint maxPlayers = peer->GetMaximumIncomingConnections();
	if ((float)teams[toTeam].members.size() >= ((float)maxPlayers / 2.0F)) {
		BitStream bs;
		bs.Write((MessageID)ID_JOIN_TEAM_FAILED_TEAM_FULL);
		peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 2, packet->systemAddress, false);
		return;
	}

	BitStream responsebitStream;
	responsebitStream.Write((MessageID)ID_JOIN_TEAM_ACCEPTED);
	responsebitStream.Write(toTeam);
	peer->Send(&responsebitStream, MEDIUM_PRIORITY, RELIABLE, 3, packet->systemAddress, false);

	changeTeam(toTeam, peer->GetIndexFromSystemAddress(packet->systemAddress));

	sendTeamUpdate();
}

void Server::sendTeamUpdate() {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_TEAM_UPDATE);
	bitStream.Write((unsigned char)teams.size());
	for (unsigned int i = 0; i < teams.size(); i++) {
		Team::serialize(bitStream, teams[(TeamId)i], true);
	}

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE_SEQUENCED, 4, UNASSIGNED_SYSTEM_ADDRESS, true);

	console::dlog("Team update sent");
}

void Server::sendAllowSpawnMsg(uchar clientId) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_CAN_SPAWN);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 5, users.at(clientId).guid, false);
}

void Server::handleShipUpdate(Packet* packet) {
	uchar clientId = peer->GetIndexFromSystemAddress(packet->systemAddress);

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	ShipState newShipState;
	ShipState::serialize(bitStream, newShipState, false);

	shipStateJitterBuffers[clientId].push_back(newShipState);

	if (shipStateJitterBuffers[clientId].size() > 2) {
		shipStateJitterBuffers[clientId].pop_front();
	}

	//console::dlog("Received ship state from " + std::to_string((int)clientId) + ", ship ypos: " + std::to_string(shipStates[clientId].position.y));
}

void Server::broadcastShipStates(ServerShipStates& newStates) {
	if (newStates.states.size() == 0)
		return;

	BitStream bitStream;
	bitStream.Write((MessageID)ID_SHIP_UPDATE);
	ServerShipStates::serialize(bitStream, newStates, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	//console::dlog("Number of ships in update: " + std::to_string(newStates.states.size()));
}

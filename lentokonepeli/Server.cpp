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
	peer->SetUnreliableTimeout(10000);
}

void Server::start(sf::Uint8 maxClients) {
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

void Server::changeTeam(TeamId newTeam, sf::Uint8 clientId) {
	TeamId oldTeam = users.at(clientId).teamId;

	auto it = std::find(teams[oldTeam].members.begin(), teams[oldTeam].members.end(), clientId);
	if (it != teams[oldTeam].members.end()) {
		teams[oldTeam].members.erase(it);
	}

	teams[newTeam].members.push_back(clientId);
	users.at(clientId).teamId = newTeam;

	game->onClientJoinTeam(clientId, newTeam);

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
		game->onUserConnect(&users[user.clientId]);

		// send data of all users to new connected user
		for (auto& pair : users) {
			sendUserUpdate(pair.second, packet->systemAddress, false);
		}
	}

	sendUserUpdate(user, packet->systemAddress, true); // send data of this user to all others

	//console::log("User update: " + std::to_string(user.clientId) + (std::string)user.username.C_String() + user.guid.ToString());
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
			
			auto teamit = std::find(teams[it->second.teamId].members.begin(), teams[it->second.teamId].members.end(), it->second.clientId);
			if (teamit != teams[it->second.teamId].members.end())
				teams[it->second.teamId].members.erase(teamit);

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

void Server::handleShipUpdate(Packet* packet) {
	sf::Uint8 clientId = peer->GetIndexFromSystemAddress(packet->systemAddress);

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);

	ShipState newShipState;
	newShipState.serialize(bitStream, false);

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

void Server::sendBulletHitShip(Bullet* bullet, Ship* targetShip) {
	BitStream bitStream;
	bitStream.Write((MessageID)ID_BULLET_HIT_SHIP);
	bitStream.Write(bullet->clientId);
	bitStream.Write(bullet->bulletId);
	bitStream.Write(targetShip->owner->clientId);
	bitStream.Write(bullet->damage);

	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

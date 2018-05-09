#include "Client.h"
#include "PacketHelper.h"
#include "Raknet\PacketLogger.h"
#include "Console.h"
#include "Master.h"
#include "ClientGame.h"

using namespace ph;

void Client::init(Master* master_, ClientGame* game_) {
	master = master_;
	game = game_;
	master->gui.client = this;

	peer = RakPeerInterface::GetInstance();

	StartupResult res = peer->Startup(1, &SocketDescriptor(), 1);
	if (res != StartupResult::RAKNET_ALREADY_STARTED && res != StartupResult::RAKNET_STARTED) {
		console::log("Raknet could not be started, error code: " + (int)res);
	}
	else {
		console::log("Client started");
	}

	for (int i = 0; i < TEAMS_SIZE; i++) {
		teams[(TeamId)i] = Team((TeamId)i);
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

void Client::start(std::string hostIp, RakString username) {
	myUsername = username;

	peer->Connect(hostIp.c_str(), SERVER_PORT, 0, 0);
	peer->SetOccasionalPing(true);
}

void Client::requestTeamJoin(TeamId toTeam) {
	console::dlog("Sending team join request: " + std::to_string(toTeam));
	BitStream bitStream;
	bitStream.Write((MessageID)ID_JOIN_TEAM_REQUEST);
	bitStream.Write((unsigned char)toTeam);
	peer->Send(&bitStream, MEDIUM_PRIORITY, RELIABLE, 0, hostguid, false);
}

void Client::update() {
	Packet* packet = peer->Receive();

	while (packet != 0) {

		MessageID packetId = getPacketIdentifier(packet);

		if (packetId == ID_CONNECTION_REQUEST_ACCEPTED) {
			console::log("Connection accepted");
			hostguid = packet->guid;

			//Send my username to server (server fills rest of user members)
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
		else if (packetId == ID_CONNECTION_ATTEMPT_FAILED) {
			console::log("Connection failed");
		}
		else if (packetId == ID_DISCONNECTION_NOTIFICATION) {
			console::log("Disconnected, server has been closed");
		}
		else if (packetId == ID_JOIN_TEAM_FAILED_TEAM_FULL) {
			console::log("Couldn't join: team full");
		}
		else if (packetId == ID_TEAM_UPDATE) {
			processTeamUpdate(packet);
		}
		else if (packetId == ID_CAN_SPAWN) {
			console::dlog("Can spawn msg received");
			game->onSpawnAllowed();
		}
		else if (packetId == ID_SHIP_UPDATE) {
			processShipUpdate(packet);
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

	lastPing = peer->GetLastPing(peer->GetSystemAddressFromIndex(0));
}

void Client::processUser(Packet* packet) {
	User user;
	unsigned char c;

	BitStream bitStream(packet->data, packet->length, false);
	bitStream.Read(c);
	User::serialize(bitStream, user, false);

	users[user.clientId] = user;

	if (user.guid == peer->GetMyGUID()) {
		myId = user.clientId;
		connectionDone = true;
		game->onConnectionComplete();
	}
	else {
		game->onOtherUserConnect(&user);
	}

	console::dlog("User update: " + std::to_string((unsigned int)user.clientId) + (std::string)user.username + user.guid.ToString());
}

void Client::handleOtherUserDisconnect(Packet* packet) {
	User user;
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	User::serialize(bitStream, user, false);
	users.erase(user.clientId);
	console::log((std::string)user.username.C_String() + " disconnected");

	game->onOtherUserDisconnect(user.clientId);
}

void Client::processTeamUpdate(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	TeamChange tc;
	tc.serialize(bitStream, false);

	TeamId oldTeam = (TeamId)tc.oldTeamId;
	TeamId newTeam = (TeamId)tc.newTeamId;
	sf::Uint8 clientId = tc.clientId;

	auto it = std::find(teams[oldTeam].members.begin(), teams[oldTeam].members.end(), clientId);
	if (it != teams[oldTeam].members.end()) {
		teams[oldTeam].members.erase(it);
	}

	teams[newTeam].members.push_back(clientId);
	users.at(clientId).teamId = newTeam;

	game->onTeamJoin(clientId, newTeam);
}

void Client::processShipUpdate(Packet* packet) {
	BitStream bitStream(packet->data, packet->length, false);
	bitStream.IgnoreBytes(1);
	ServerShipStates sss;
	ServerShipStates::serialize(bitStream, sss, false);

	serverStateJitterBuffer.push_back(sss);

	if (serverStateJitterBuffer.size() > jitterBufferMaxSize) {
		serverStateJitterBuffer.pop_front();
	}

	//console::dlog("Ship update received");
}

void Client::sendShipUpdate(ShipState& shipState) {
	
	BitStream bitStream;
	bitStream.Write((MessageID)ID_SHIP_UPDATE);
	shipState.serialize(bitStream, true);

	peer->Send(&bitStream, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, hostguid, false);

	tickCount++;
}
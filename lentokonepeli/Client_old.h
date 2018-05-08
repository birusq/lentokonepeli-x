#pragma once

#include "SFML\Network.hpp"
#include "PacketHelper.h"
#include <queue>
#include <unordered_set>

class Client {
public:
	void start(sf::IpAddress hostAddress_);

	void startConnecting();

	void bindToFreePort(sf::UdpSocket& socket_);

	void handleConnectionResponse(sf::Packet& response);

	void sendStateToServer(float dt, ClientState& cs);

	void updateServerStates(float dt);

	void disconnect();

	sf::Uint8 clientIndex;

	sf::Uint16 remoteSequenceNumber = 0;
	sf::Uint16 localSequenceNumber = 0;

	std::deque<std::pair<sf::Uint16, ServerState>> serverStateBuffer;
	std::unordered_set<sf::Uint16> receivedRemoteSeqNums;
	sf::Uint16 lastProcessedRemoteSeqNum = 0;

	bool isBufferEnough();

	const int bufferSize = 4;

private:

	void Client::handleReliabilityOnReceive(sf::Packet& packet, sf::Uint16 newRemoteSeqNum);
	std::bitset<32> Client::createAckSet();

	std::unordered_map<sf::Uint16, bool> remoteReceivedSeqNums;

	bool bufferDone = false;

	

	sf::UdpSocket socket;
	sf::IpAddress hostAddress;
	unsigned int startingPort = 65000;
	unsigned int hostPort = 65000;

	sf::Int32 clientSalt;
	sf::Int32 serverSalt;
};
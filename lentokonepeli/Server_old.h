#pragma once

#include <SFML\Network.hpp>
#include <unordered_map>
#include "PhysicsTransformable.h"
#include "PacketHelper.h"
#include <deque>
#include <bitset>
#include <unordered_set>


class Server {
public:
	Server() {};
	void start(sf::Uint8 maxClients_);
	void update(float dt);
	void broadcastChanges(ServerState& ss);

	int findFreeClientIndex();
	int findExistingClientIndex(const sf::IpAddress& address, unsigned short portNum);
	bool isConnected(sf::Uint8 clientIndex);
	sf::IpAddress getAddress(sf::Uint8 clientIndex);
	unsigned short getPort(sf::Uint8 clientIndex);

	std::vector<std::deque<std::pair<sf::Uint16, ClientState>>> clientStateBuffers;
	std::vector<std::unordered_set<sf::Uint16>> receivedRemoteSeqNums;
	std::vector<sf::Uint16> lastProcessedRemoteSeqNum;

	int maxClients;

	sf::Uint16 serverSequenceNumber = 0;

private:
	void respondToConnectRequest(sf::Packet& packet, sf::IpAddress& senderAddress, unsigned short senderPort);
	void sendDenyConnectionPacket(sf::IpAddress& toAddress, unsigned short senderPort, bool requestPortChange);
	void sendAcceptConnectionPacket(sf::IpAddress& toAddress, sf::Uint8 clientIndex);

	void disconnectClient(sf::Uint8 clientIndex);

	void handleReliabilityOnReceive(sf::Packet& packet, sf::Uint16 newRemoteSeqNum, sf::Uint8 clientIndex);
	std::bitset<32> createAckSet(sf::Uint8 clientIndex);

	int totalConnectedClients;
	std::vector<bool> clientsConnected;
	std::vector<sf::IpAddress> clientAddresses;
	std::vector<sf::Int32> clientSalts;
	std::vector<sf::Int32> serverSalts;
	std::vector<float> clientTimeoutTimers;
	std::vector<unsigned short> clientPorts;
	std::vector<sf::Uint16> remoteSequenceNumbers;
	std::vector<std::unordered_map<sf::Uint16, bool>> remoteReceivedSeqNums;


	sf::UdpSocket socket;

	unsigned short defaultPort = 65000;

	const float connectionTimeout = 5.0F;
	
	PacketHelper ph;
};


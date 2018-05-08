#pragma once

#include "Server.h"
#include <iostream>
#include <Thor\Math.hpp>

void Server::start(sf::Uint8 maxClients_) {

	maxClients = maxClients_;
	clientsConnected.resize(maxClients, false);
	clientAddresses.resize(maxClients, sf::IpAddress::None);
	clientSalts.resize(maxClients, sf::Int32(0));
	serverSalts.resize(maxClients, sf::Int32(0));
	clientTimeoutTimers.resize(maxClients, 0.0F);
	clientPorts.resize(maxClients, 65000);
	clientStateBuffers.resize(maxClients);
	remoteSequenceNumbers.resize(maxClients, sf::Uint16(0));
	remoteReceivedSeqNums.resize(maxClients);
	receivedRemoteSeqNums.resize(maxClients);
	lastProcessedRemoteSeqNum.resize(maxClients);

	socket.bind(defaultPort);
	socket.setBlocking(false);

	std::cout << "Server opened\n";
}

int Server::findFreeClientIndex() {
	for (int i = 0; i < maxClients; ++i) {
		if (clientsConnected[i] == false)
			return i;
	}
	return -1;
}

int Server::findExistingClientIndex(const sf::IpAddress& address, unsigned short portNum) {
	for (int i = 0; i < maxClients; ++i) {
		if (clientsConnected[i] == true && clientAddresses[i] == address && clientPorts[i] == portNum)
			return i;
	}
	return -1;
}

bool Server::isConnected(sf::Uint8 clientIndex) {
	return clientsConnected[clientIndex];
}

sf::IpAddress Server::getAddress(sf::Uint8 clientIndex) {
	return clientAddresses[clientIndex];
}

unsigned short Server::getPort(sf::Uint8 clientIndex) {
	return clientPorts[clientIndex];
}


void Server::update(float dt) {
	sf::Packet packet;
	sf::IpAddress senderAddress;
	unsigned short senderPort;
	sf::Int8 packetType;

	while (socket.receive(packet, senderAddress, senderPort) == sf::Socket::Done) {

		packet >> packetType;
		
		if (packetType == PacketHelper::ConnectionRequest) {
			respondToConnectRequest(packet, senderAddress, senderPort);
		}
		else if (packetType == PacketHelper::ConnectionPayload || packetType == PacketHelper::DisconnectRequest) {

			sf::Int32 xorSalt;
			packet >> xorSalt;

			int clientIndex = findExistingClientIndex(senderAddress, senderPort);
			
			if (clientIndex != -1 && xorSalt == (clientSalts.at(clientIndex) ^ serverSalts.at(clientIndex))) {
				if (packetType == PacketHelper::ConnectionPayload) {

					sf::Uint16 newRemoteSeqNum;
					packet >> newRemoteSeqNum;

					handleReliabilityOnReceive(packet, newRemoteSeqNum, clientIndex);

					ClientState cs;
					packet >> cs;
					clientStateBuffers[clientIndex].push_back(std::make_pair(newRemoteSeqNum, cs));
					//std::cout << "Payload packet received\n";
				}
				if (packetType == PacketHelper::DisconnectRequest) {
					std::cout << "Disconnected client " << (int)clientIndex << " because it sent a disconnect request\n";
					disconnectClient(clientIndex);
				}

				clientTimeoutTimers[findExistingClientIndex(senderAddress, senderPort)] = 0.0F;
			}
		}
	}

	for (int i = 0; i < maxClients; ++i) {
		if (clientsConnected[i] == true) {
			
			clientTimeoutTimers[i] += dt;
			if (clientTimeoutTimers[i] > 5.0F) {
				disconnectClient(i);
			}
		}
	}
}

void Server::broadcastChanges(ServerState& ss) {
	for (int i = 0; i < maxClients; ++i) {
		if (clientsConnected[i] == true) {
			sf::Packet packet;

			packet << sf::Int8(PacketHelper::ConnectionPayload);
			packet << (clientSalts[i] ^ serverSalts[i]);
			packet << serverSequenceNumber;
			packet << remoteSequenceNumbers[i];
			packet << sf::Int32(createAckSet(i).to_ulong());
			packet << ss;

			if (socket.send(packet, clientAddresses[i], clientPorts[i]) != sf::Socket::Done) {
				std::cerr << "Error sending payload packet\n";
			}
			else {
				//std::cout << "Payload packet "<< serverSequenceNumber << " sent\n";
			}
		}
	}
	++serverSequenceNumber;
}

void Server::respondToConnectRequest(sf::Packet& packet, sf::IpAddress& senderAddress, unsigned short senderPort) {
	int clientIndex = findFreeClientIndex();

	std::cout << "ClientIndex " << (int)clientIndex << " is free\n";

	if (clientIndex == -1) {
		sendDenyConnectionPacket(senderAddress, senderPort, false);
	}
	else {
		auto it = std::find(clientAddresses.begin(), clientAddresses.end(), senderAddress);

		if (it != clientAddresses.end()) { // address taken
			if (clientPorts[it - clientAddresses.begin()] == senderPort) { // address holder also has same port, request new port
				sendDenyConnectionPacket(senderAddress, senderPort, true);
				return;
			}
		}

		packet >> clientSalts[clientIndex];

		serverSalts[clientIndex] = thor::random(INT_MIN, INT_MAX);
		
		clientsConnected[clientIndex] = true;
		clientAddresses[clientIndex] = senderAddress;
		clientPorts[clientIndex] = senderPort;

		sendAcceptConnectionPacket(senderAddress, clientIndex);
	}
}

void Server::sendDenyConnectionPacket(sf::IpAddress& toAddress, unsigned short senderPort, bool requestPortChange) {
	sf::Packet packet;

	packet << sf::Int8(PacketHelper::ConnectionResponse);
	packet << false;
	packet << requestPortChange;

	if (socket.send(packet, toAddress, senderPort) != sf::Socket::Done) {
		std::cerr << "Error sending rejection packet\n";
	}
	else {
		std::cout << "Client rejected\n";
	}
}

void Server::sendAcceptConnectionPacket(sf::IpAddress& toAddress, sf::Uint8 clientIndex) {
	sf::Packet packet;

	packet << sf::Int8(PacketHelper::ConnectionResponse);
	packet << true;
	packet << clientSalts[clientIndex];
	packet << serverSalts[clientIndex];
	packet << (sf::Uint8)clientIndex;
	packet << serverSequenceNumber;

	if (socket.send(packet, toAddress, clientPorts[clientIndex]) != sf::Socket::Done) {
		std::cerr << "Error sending accept packet\n";
	}
	else {
		std::cout << "Client " << (int)clientIndex << " connected\n";
	}
}

void Server::disconnectClient(sf::Uint8 clientIndex) {
	std::cout << "client " << (int)clientIndex << " disconnected\n";
	clientsConnected[clientIndex] = false;
	clientAddresses[clientIndex] = sf::IpAddress::None;
	clientPorts[clientIndex] = 0;
	clientTimeoutTimers[clientIndex] = 0.0F;
	clientStateBuffers[clientIndex] = std::deque<std::pair<sf::Uint16, ClientState>>();
	remoteSequenceNumbers[clientIndex] = sf::Uint16(0);
	remoteReceivedSeqNums[clientIndex] = std::unordered_map<sf::Uint16, bool>();
	receivedRemoteSeqNums[clientIndex] = std::unordered_set<sf::Uint16>();
	lastProcessedRemoteSeqNum[clientIndex] = sf::Uint16(0);
}

void Server::handleReliabilityOnReceive(sf::Packet& packet, sf::Uint16 newRemoteSeqNum, sf::Uint8 clientIndex) {
	
	if (PacketHelper::sequenceGreaterThan(newRemoteSeqNum, remoteSequenceNumbers[clientIndex])) {
		remoteSequenceNumbers[clientIndex] = newRemoteSeqNum;
	}

	receivedRemoteSeqNums[clientIndex].insert(newRemoteSeqNum);

	sf::Uint16 ack;
	packet >> ack;

	sf::Uint32 temp_ackBitset;
	packet >> temp_ackBitset;
	std::bitset<32> ackBitset(temp_ackBitset);

	remoteReceivedSeqNums[clientIndex][ack] = true;

	for (int i = 0; i < 32; ++i) {
		remoteReceivedSeqNums[clientIndex][ack - i - 1] |= ackBitset.test(i);
	}

	std::vector<sf::Uint16> itemsToErase;

	for (auto& pair : remoteReceivedSeqNums[clientIndex]) {
		if (pair.first < ack - 33) {
			if (pair.second == false) {
				std::cout << "Client " << clientIndex << " lost packet " << pair.first;
			}
			itemsToErase.push_back(pair.first);
		}
	}

	for (sf::Uint16 item : itemsToErase) {
		remoteReceivedSeqNums[clientIndex].erase(item);
	}
}

std::bitset<32> Server::createAckSet(sf::Uint8 clientIndex) {
	std::bitset<32> ackSet;
	sf::Uint16 startAck = remoteSequenceNumbers[clientIndex];
	for (int i = 0; i < 32; ++i) {
		ackSet[i] = (receivedRemoteSeqNums[clientIndex].count(startAck - i - 1));
	}
	return ackSet;
}

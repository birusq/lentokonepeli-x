#include "Client.h"
#include <iostream>
#include <Thor\Math.hpp>

void Client::start(sf::IpAddress hostAddress_) {
	hostAddress = hostAddress_;
	
	bindToFreePort(socket);

	socket.setBlocking(false);

	std::cout << "Client opened\n";

	startConnecting();
}

void Client::bindToFreePort(sf::UdpSocket& sock) {
	bool done = false;
	unsigned short bindingPort = startingPort;
	while (done == false) {
		if (sock.bind(bindingPort) == sf::Socket::Done) {
			done = true;
			std::cout << "Socket binded to port " << bindingPort << "\n";
		}
		else if (bindingPort < USHRT_MAX){
			++bindingPort;
		}
		else {
			std::cerr << "couldn't bind socket to any port\n";
		}
	}
}

void Client::startConnecting() {
	bool gotResponse = false;

	int tryCount = 10;
	while (gotResponse == false && tryCount > 0) {
		tryCount--;

		sf::Packet packet;

		clientSalt = thor::random(INT_MIN, INT_MAX);

		packet << sf::Int8(PacketHelper::ConnectionRequest);
		packet << clientSalt;

		std::cout << "Sending connection request\n";
		if (socket.send(packet, hostAddress, hostPort) != sf::Socket::Done) {
			std::cout << "Error sending connection request\n";
		}
		
		sf::sleep(sf::seconds(0.5F));

		sf::Packet receivedPacket;
		sf::IpAddress senderAddress;
		unsigned short senderPort;
		if (socket.receive(receivedPacket, senderAddress, senderPort) != sf::Socket::Done) {
			std::cout << "Didn't get connection response\nTrying again...\n";
		}
		else {
			sf::Int8 pt;
			receivedPacket >> pt;
			if (pt == PacketHelper::ConnectionResponse) {
				gotResponse = true;
				handleConnectionResponse(receivedPacket);
			}
			else {
				std::cout << "Didn't get connection response\nTrying again...\n";
			}
		}
	}
}

void Client::handleConnectionResponse(sf::Packet& packet) {
	
	bool accepted;
	packet >> accepted;

	if (accepted) {
		std::cout << "Connection accepted\n";
		packet >> clientSalt;
		packet >> serverSalt;
		packet >> clientIndex;
		packet >> lastProcessedRemoteSeqNum;

	}
	else {
		bool tryNextPort;
		packet >> tryNextPort;

		unsigned short newPort = socket.getLocalPort() + 1;
		if (tryNextPort == true && newPort > 64999) {
			socket.bind(newPort);
			std::cout << "Trying to connect with port " << newPort << "\n";
			startConnecting();
		}
		else {
			std::cout << "Connection denied\n";
		}
	}
}

void Client::sendStateToServer(float dt, ClientState& clientState) {
	sf::Packet packet;

	packet << sf::Int8(PacketHelper::ConnectionPayload);
	packet << (clientSalt ^ serverSalt);
	packet << localSequenceNumber;
	packet << remoteSequenceNumber;
	packet << sf::Uint16(createAckSet().to_ulong());
	packet << clientState;

	if (socket.send(packet, hostAddress, hostPort) != sf::Socket::Done) {
		std::cout << "Error sending packet\n";
	}
	else {
		//std::cout << "Payload packet sent\n";
	}
	localSequenceNumber++;
}

void Client::updateServerStates(float dt) {
	sf::Packet packet;
	sf::IpAddress senderAddress;
	unsigned short senderPort;
	sf::Int8 packetType;

	while (socket.receive(packet, senderAddress, senderPort) == sf::Socket::Done) {
		packet >> packetType;

		if (packetType == PacketHelper::ConnectionPayload) {

			sf::Int32 xorSalt;
			packet >> xorSalt;

			if (xorSalt == (clientSalt ^ serverSalt)) {
				
				sf::Uint16 newRemoteSeqNum;
				packet >> newRemoteSeqNum;

				handleReliabilityOnReceive(packet, newRemoteSeqNum);

				ServerState ss;
				packet >> ss;
				serverStateBuffer.push_back(std::make_pair(newRemoteSeqNum, ss));
			}
			else {
				std::cout << "Wrong salt in payload packet\n";
			}
		}
	}
}

bool Client::isBufferEnough() {
	if (bufferDone) {
		return true;
	}
	else {
		if (serverStateBuffer.size() >= (size_t)bufferSize) {
			bufferDone = true;
			return true;
		}
		else {
			return false;
		}
	}
}

void Client::disconnect() {
	sf::Packet packet;

	packet << sf::Int8(PacketHelper::DisconnectRequest);
	packet << (clientSalt ^ serverSalt);

	if (socket.send(packet, hostAddress, hostPort) != sf::Socket::Done) {
		std::cout << "Error sending disconnect request\n";
	}
}

void Client::handleReliabilityOnReceive(sf::Packet& packet, sf::Uint16 newRemoteSeqNum) {
	if (PacketHelper::sequenceGreaterThan(newRemoteSeqNum, remoteSequenceNumber)) {
		remoteSequenceNumber = newRemoteSeqNum;
	}

	receivedRemoteSeqNums.insert(newRemoteSeqNum);

	sf::Uint16 ack;
	packet >> ack;

	sf::Uint32 temp_ackBitset;
	packet >> temp_ackBitset;
	std::bitset<32> ackBitset(temp_ackBitset);

	remoteReceivedSeqNums[ack] = true;

	for (int i = 0; i < 32; ++i) {
		remoteReceivedSeqNums[ack - i - 1] |= ackBitset.test(i);
	}

	std::vector<sf::Uint16> itemsToErase;

	for (auto& pair : remoteReceivedSeqNums) {
		if (pair.first < ack - 33) {
			if (pair.second == false) {
				std::cout << "Server lost packet " << pair.first;
			}
			itemsToErase.push_back(pair.first);
		}
	} 

	for (sf::Uint16 item : itemsToErase) {
		remoteReceivedSeqNums.erase(item);
	}
}

std::bitset<32> Client::createAckSet() {
	std::bitset<32> ackSet;
	sf::Uint16 startAck = remoteSequenceNumber;
	for (int i = 0; i < 32; ++i) {
		ackSet[i] = (receivedRemoteSeqNums.count(startAck - i - 1));
	}
	return ackSet;
}
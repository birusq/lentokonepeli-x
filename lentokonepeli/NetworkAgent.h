#pragma once

#include "User.h"
#include "Team.h"
#include "RakNet/RakPeerInterface.h"

class NetworkAgent {
public: 
	std::unordered_map<sf::Uint8, User> users;
	std::unordered_map<Team::Id, Team> teams;

	bool isConnected(sf::Uint8 clientId) { return users.count(clientId) == 1; }

	virtual bool isMyId(sf::Uint8 clientId) = 0;

	virtual void update() = 0;

	virtual void close() = 0;

protected: 
	RakNet::RakPeerInterface* peer = nullptr;
};
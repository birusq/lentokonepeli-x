#pragma once

#include "Team.h"
#include "Raknet\BitStream.h"
#include "SFML\System.hpp"

struct User {
	static void serialize(RakNet::BitStream& bitStream, User& user, bool write);

	unsigned char clientId;
	RakNet::RakNetGUID guid;
	RakNet::RakString username;
};
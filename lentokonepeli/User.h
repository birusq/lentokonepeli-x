#pragma once

#include "Team.h"

struct User {
	static void serialize(RakNet::BitStream& bitStream, User& user, bool write);

	unsigned char clientId;
	RakNet::RakNetGUID guid;
	RakNet::RakString username;

	Team::Id teamId;
};
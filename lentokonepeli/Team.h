#pragma once

#include "Raknet\BitStream.h"
#include <vector>

enum TeamId : unsigned char {
	NO_TEAM,
	RED_TEAM,
	BLUE_TEAM,
	TEAMS_SIZE
};

class Team {
public:
	Team() {}
	Team(TeamId id_) : id{ id_ } {}

	static void serialize(RakNet::BitStream& bitStream, Team& team, bool write);

	unsigned char id;

	std::vector<unsigned char> members;
};


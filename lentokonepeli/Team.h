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

	unsigned char id;

	std::vector<unsigned char> members;
};

struct TeamChange {

	TeamChange() {}
	TeamChange(unsigned char oldTeamId_, unsigned char newTeamId_, unsigned char clientId_) : oldTeamId{ oldTeamId_ }, newTeamId{ newTeamId_ }, clientId{ clientId_ } {}

	void serialize(RakNet::BitStream& bitStream, bool write);

	unsigned char oldTeamId;
	unsigned char newTeamId;
	unsigned char clientId;
};


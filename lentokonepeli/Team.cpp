#include "Team.h"

void Team::serialize(RakNet::BitStream& bitStream, Team& team, bool write) {
	bitStream.Serialize(write, team.id);
	
	unsigned char memberCount = (unsigned char)team.members.size();
	bitStream.Serialize(write, memberCount);

	if (!write) {
		team.members.clear();	
	}

	for (int i = 0; i < memberCount; i++) {
		if (write) {
			bitStream.Write(team.members.at(i));
		}
		else { // read
			unsigned char member;
			bitStream.Read(member);
			team.members.push_back(member);
		}
	}
}

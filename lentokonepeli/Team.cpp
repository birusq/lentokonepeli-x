#include "Team.h"

void TeamChange::serialize(RakNet::BitStream& bitStream, bool write) {
	bitStream.Serialize(write, oldTeamId);
	bitStream.Serialize(write, newTeamId);
	bitStream.Serialize(write, clientId);
}
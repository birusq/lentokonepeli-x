#include "Team.h"

void Team::removeClient(sf::Uint8 clientId) {
	auto it = std::find(members.begin(), members.end(), clientId);
	if (it != members.end()) {
		members.erase(it);
	}
}

void TeamChange::serialize(RakNet::BitStream& bitStream, bool write) {
	bitStream.Serialize(write, oldTeamId);
	bitStream.Serialize(write, newTeamId);
	bitStream.Serialize(write, clientId);
}
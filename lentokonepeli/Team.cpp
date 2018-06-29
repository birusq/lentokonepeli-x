#include "Team.h"
#include "Globals.h"

void Team::removeClient(sf::Uint8 clientId) {
	auto it = std::find(members.begin(), members.end(), clientId);
	if (it != members.end()) {
		members.erase(it);
	}
}

sf::Color Team::getColor() {
	if((Id)id == Team::RED_TEAM) {
		return palette::red;
	}
	else if((Id)id == Team::BLUE_TEAM) {
		return palette::blue;
	}
	return sf::Color::White;
}

void TeamChange::serialize(RakNet::BitStream& bitStream, bool write) {
	bitStream.Serialize(write, oldTeamId);
	bitStream.Serialize(write, newTeamId);
	bitStream.Serialize(write, clientId);
}
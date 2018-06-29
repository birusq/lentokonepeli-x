#include "Scoreboard.h"

void KillDetails::serialize(RakNet::BitStream& bitStream, bool write) {
	bitStream.Serialize(write, clientKilled);
	sf::Uint8 size = (sf::Uint8)contributors.size();
	bitStream.Serialize(write, size);

	if(write == false) {
		contributors.clear();
	}

	for(sf::Uint8 i = 0; i < size; i++) {
		if(write) {
			bitStream.Write(contributors[i].clientId);
			bitStream.Write(contributors[i].dmg);
		}
		else {
			contributors.push_back(DmgContributor());
			bitStream.Read(contributors[i].clientId);
			bitStream.Read(contributors[i].dmg);
		}
	}
}

void Scoreboard::addKillDetails(KillDetails& killDetails) {
	for(std::size_t i = 0; i < killDetails.contributors.size(); i++) {
		sf::Uint8 clientId = killDetails.contributors[i].clientId;
		if(i == 0) {
			userScores[clientId].kills += 1;
			userScores[clientId].points += 100;
		}
		else {
			userScores[clientId].assists += 1;
			userScores[clientId].points += killDetails.contributors[i].dmg;
		}
	}
}

void Scoreboard::deleteClient(sf::Uint8 clientId) {
	if(userScores.count(clientId) == 1) {
		userScores.erase(clientId);
	}
	auto it = std::find(sortedUsers.begin(), sortedUsers.end(), clientId);
	if(it != sortedUsers.end()) {
		sortedUsers.erase(it);
	}
}

void Scoreboard::serialize(RakNet::BitStream & bitStream, bool write) {
	
}


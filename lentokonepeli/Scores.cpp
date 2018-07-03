#include "Scores.h"
#include "NetworkAgent.h"
#include <algorithm>
#include "Server.h"
#include "Master.h"
#include "GUIScoreboard.h"

std::ostream & operator<<(std::ostream & str, DmgContributor & c) {
	str << "DmgContributor: " << (int)c.clientId << "\n";
	str << "dmg: " << (int)c.dmg;
	str << " timer: " << c.timer.getTimeRemaining() << "\n";
	return str;
}


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

	console::dlogStream();
}

std::ostream & operator<<(std::ostream & str, KillDetails & c) {
	str << "KillDetails: " << (int)c.clientKilled << "\n";
	for(DmgContributor contributor : c.contributors) {
		str << contributor;
	}
	return str;
}

void UserScore::serialize(RakNet::BitStream & bitStream, bool write) {
	bitStream.Serialize(write, kills);
	bitStream.Serialize(write, deaths);
	bitStream.Serialize(write, assists);
	bitStream.Serialize(write, points);
}

void Scores::init(NetworkAgent* agent) {
	networkAgent = agent;
}

void Scores::addKillDetails(KillDetails& killDetails) {
	if(userScores.count(killDetails.clientKilled) == 1) {
		userScores[killDetails.clientKilled].deaths += 1;
	}
	for(std::size_t i = 0; i < killDetails.contributors.size(); i++) {
		sf::Uint8 clientId = killDetails.contributors[i].clientId;
		if(userScores.count(clientId) == 1) {
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
	sortUsers();

	if(master->gui.scoreBoard != nullptr) {
		master->gui.scoreBoard->updateScores(userScores, sortedUsers, *networkAgent);
	}
}

void Scores::addUser(const User& user) {
	// Delete this just in case to avoid duplicates
	deleteUser(user);

	userScores[user.clientId] = UserScore();
	sortedUsers[user.teamId].push_back(user.clientId);

	if(master->gui.scoreBoard != nullptr) {
		master->gui.scoreBoard->updateScores(userScores, sortedUsers, *networkAgent);
	}
}

void Scores::deleteUser(const User& user) {
	if(userScores.count(user.clientId) == 1) {
		userScores.erase(user.clientId);

		for(int i = 1; i < Team::TEAMS_SIZE; i++) {
			if(sortedUsers[(Team::Id)i].size() != 0) {
				auto it = std::find(sortedUsers[(Team::Id)i].begin(), sortedUsers[(Team::Id)i].end(), user.clientId);
				if(it != sortedUsers[(Team::Id)i].end()) {
					sortedUsers[(Team::Id)i].erase(it);
				}
			}
		}

		if(master->gui.scoreBoard != nullptr) {
			master->gui.scoreBoard->updateScores(userScores, sortedUsers, *networkAgent);
		}
	}
}

void Scores::sortUsers() {
	for(int i = 1; i < Team::TEAMS_SIZE; i++) {
		std::sort(sortedUsers[(Team::Id)i].begin(), sortedUsers[(Team::Id)i].end(), [&](const sf::Uint8& lhs, const sf::Uint8& rhs) {
			return userPointsCompare(lhs, rhs);
		});
	}
}


void Scores::serialize(RakNet::BitStream& bitStream, bool write) {
	if(write == false) {
		userScores.clear();
	}
	for(int i = 1; i < Team::TEAMS_SIZE; i++) {
		Team::Id teamId = (Team::Id)i;

		sf::Uint8 userCount = (sf::Uint8)sortedUsers[teamId].size();
		bitStream.Serialize(write, userCount);

		if(write == false) {
			sortedUsers[teamId].clear();
		}

		for(sf::Uint8 j = 0; j < userCount; j++) {
			if(write == true) {
				sf::Uint8 clientId = sortedUsers[teamId][j];
				bitStream.Write(clientId);
				bitStream.Write(userScores[clientId]);
			}
			else {
				sf::Uint8 clientId;
				bitStream.Read(clientId);
				sortedUsers[teamId].push_back(clientId);
				UserScore score;
				bitStream.Read(score);
				userScores[clientId] = score;
			}
		}
	}
}

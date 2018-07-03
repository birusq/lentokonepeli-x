#pragma once

#include <unordered_map>
#include <SFML/System.hpp>
#include "Raknet\BitStream.h"
#include "CountdownTimer.h"
#include "Team.h"
#include "User.h"

struct DmgContributor {
	DmgContributor() {}
	DmgContributor(sf::Uint8 clientId_, sf::Int16 dmg_, CountdownTimer timer_) : clientId{ clientId_ }, dmg{ dmg_ }, timer{ timer_ } {}
	sf::Uint8 clientId;
	sf::Int16 dmg;
	CountdownTimer timer;

	friend std::ostream& operator<<(std::ostream & str, DmgContributor & c);
};

struct KillDetails {
	void serialize(RakNet::BitStream& bitStream, bool write);

	friend std::ostream& operator<<(std::ostream & str, KillDetails & c);

	sf::Uint8 clientKilled;
	std::vector<DmgContributor> contributors;
};

struct UserScore {
	void serialize(RakNet::BitStream& bitStream, bool write);

	sf::Int32 kills;
	sf::Int32 deaths;
	sf::Int32 assists;
	sf::Int32 points;
};

class NetworkAgent;
class GUIScoreboard;

class Scores {
public:
	void init(NetworkAgent* agent);
	void addKillDetails(KillDetails& killDetails);
	void addUser(const User& user);
	void deleteUser(const User& user);
	void serialize(RakNet::BitStream& bitStream, bool write);
private:
	std::unordered_map<sf::Uint8, UserScore> userScores;
	// Sorted based on the amount of points each user has
	std::unordered_map<Team::Id, std::vector<sf::Uint8>> sortedUsers;

	NetworkAgent* networkAgent;

	GUIScoreboard* guiScoreboard;

	void sortUsers();

	bool userPointsCompare(const sf::Uint8& c1, const sf::Uint8& c2) { return userScores[c1].points > userScores[c2].points; }
};


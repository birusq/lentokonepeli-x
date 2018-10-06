#pragma once

class Team {
public:
	enum Id : sf::Uint8 {
		NO_TEAM,
		RED_TEAM,
		BLUE_TEAM,
		TEAMS_SIZE
	};

	Team() {}
	Team(Team::Id id_) : id{ id_ } {}

	void removeClient(sf::Uint8 clientId);

	sf::Uint8 id;

	std::vector<sf::Uint8> members;

	sf::Color getColor();
};

struct TeamChange {

	TeamChange() {}
	TeamChange(sf::Uint8 oldTeamId_, sf::Uint8 newTeamId_, sf::Uint8 clientId_) : oldTeamId{ oldTeamId_ }, newTeamId{ newTeamId_ }, clientId{ clientId_ } {}

	void serialize(RakNet::BitStream& bitStream, bool write);

	sf::Uint8 oldTeamId;
	sf::Uint8 newTeamId;
	sf::Uint8 clientId;
};


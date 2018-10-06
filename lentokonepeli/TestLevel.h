#pragma once

#include "Collider.h"
#include "Team.h"

class TestLevel {
public:
	TestLevel();

	std::vector<std::shared_ptr<Collider<sf::ConvexShape>>> getGroundColliders() {};

	std::unordered_map<Team::Id, sf::Vector2f> spawnPoints;
	std::unordered_map<Team::Id, Collider<sf::RectangleShape>> spawnPointColliders;

	sf::Vector2f center;

	void draw(sf::RenderTarget& target, bool minimapPass);

	float respawnTime = 2.0F;

	float height;
	float width;

	float borderWidth;

private:

	sf::Texture bgTex;
	sf::Sprite bg;
	sf::RectangleShape mmBG;
};
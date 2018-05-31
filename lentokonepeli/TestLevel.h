#pragma once

#include "SFML\Graphics.hpp"
#include "Collider.h"
#include <vector>
#include "Team.h"
#include <unordered_map>

class TestLevel {
public:
	TestLevel();

	std::vector<Collider<sf::ConvexShape>*> getGroundColliders() {};

	std::unordered_map<Team::Id, sf::Vector2f> spawnPoints;
	std::unordered_map<Team::Id, Collider<sf::RectangleShape>> spawnPointColliders;

	sf::Vector2f center;

	void draw(sf::RenderTarget& target);

	float respawnTime = 4.0F;

private:

	sf::Texture bgTex;
	sf::Sprite bg;

	sf::RectangleShape ground;
};
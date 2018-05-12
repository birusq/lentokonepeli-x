#pragma once

#include "SFML\Graphics.hpp"
#include "Collider.h"
#include <vector>
#include "Team.h"
#include <unordered_map>

class TestLevel : public sf::Drawable {
public:
	TestLevel();

	std::vector<Collider<sf::ConvexShape>*> getColliders() {};

	std::unordered_map<TeamId, sf::Vector2f> spawnPoints;

	sf::Vector2f center;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::Texture bgTex;
	sf::Sprite bg;

	sf::RectangleShape ground;
};
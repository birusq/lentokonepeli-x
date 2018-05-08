#pragma once

#include "SFML\Graphics.hpp"
#include "Collider.h"
#include <vector>

class TestLevel : public sf::Drawable {
public:
	TestLevel();

	std::vector<Collider*> getColliders() {};

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::Texture bgTex;
	sf::Sprite bg;

	sf::RectangleShape tenUnits;

	sf::RectangleShape ground;
};
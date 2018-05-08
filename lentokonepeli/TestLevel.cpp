#include "TestLevel.h"
#include <iostream>

TestLevel::TestLevel() {
	if (!bgTex.loadFromFile("res/blue-clouds.jpg")) {
		std::cout << "Could not load bg texture";
	}

	bg.setTexture(bgTex);

	bg.setOrigin(sf::Vector2f(bgTex.getSize())/ 2.0F);
	bg.setScale(sf::Vector2f(0.15F, 0.15F));

	ground.setFillColor(sf::Color(1, 137, 10));
	ground.setSize(sf::Vector2f(1000, 30));
	ground.setPosition(0, 140);

}

void TestLevel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(bg, states);
	target.draw(ground, states);
}
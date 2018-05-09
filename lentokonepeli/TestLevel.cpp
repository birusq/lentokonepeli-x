#include "TestLevel.h"
#include <iostream>

TestLevel::TestLevel() {
	if (!bgTex.loadFromFile("res/blue-clouds.jpg")) {
		std::cout << "Could not load bg texture";
	}

	bg.setTexture(bgTex);

	bg.setTextureRect(sf::IntRect(sf::Vector2i(), sf::Vector2i(3000, 2000)));

	bg.setScale(sf::Vector2f(0.2F, 0.1F));
	bg.setPosition(sf::Vector2f(0,0));

	ground.setFillColor(sf::Color(1, 137, 10));
	ground.setSize(sf::Vector2f(600, 30));
	ground.setPosition(0, 200);

}

void TestLevel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(bg, states);
	target.draw(ground, states);
}
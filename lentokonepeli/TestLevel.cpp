#include "TestLevel.h"
#include <iostream>
#include "Console.h"

TestLevel::TestLevel() {
	if (!bgTex.loadFromFile("res/images/blue-clouds.jpg")) {
		console::dlog("Could not load bg texture");
	}

	bg.setTexture(bgTex);

	bg.setTextureRect(sf::IntRect(sf::Vector2i(), sf::Vector2i(3000, 2000)));

	bg.setScale(sf::Vector2f(0.2F, 0.1F));
	bg.setPosition(sf::Vector2f(0,0));

	ground.setFillColor(sf::Color(1, 137, 10));
	ground.setSize(sf::Vector2f(400, 30));
	ground.setPosition(0, 200);

	center = sf::Vector2f(200, 100);

	spawnPoints[RED_TEAM] = sf::Vector2f(50, 190);
	spawnPoints[BLUE_TEAM] = sf::Vector2f(150, 190);
}

void TestLevel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(bg, states);
	target.draw(ground, states);
}
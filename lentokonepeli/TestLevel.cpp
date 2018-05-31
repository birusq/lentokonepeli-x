#include "TestLevel.h"
#include <iostream>
#include "Console.h"
#include "Globals.h"

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

	spawnPoints[Team::RED_TEAM] = sf::Vector2f(50, 190);
	spawnPoints[Team::BLUE_TEAM] = sf::Vector2f(150, 190);

	sf::RectangleShape rs(sf::Vector2f(30, 20));
	rs.setOrigin(rs.getSize()/2.0F);
	rs.setOutlineThickness(0.2F);
	rs.setFillColor(sf::Color::Transparent);
	Collider<sf::RectangleShape> col;
	col.hitbox = rs;
	col.hitboxDisabled = false;

	spawnPointColliders[Team::RED_TEAM] = col;
	spawnPointColliders[Team::RED_TEAM].hitbox.setPosition(spawnPoints[Team::RED_TEAM]);
	spawnPointColliders[Team::RED_TEAM].hitbox.setOutlineColor(palette::red);

	spawnPointColliders[Team::BLUE_TEAM] = col;
	spawnPointColliders[Team::BLUE_TEAM].hitbox.setPosition(spawnPoints[Team::BLUE_TEAM]);
	spawnPointColliders[Team::BLUE_TEAM].hitbox.setOutlineColor(palette::blue);
}

void TestLevel::draw(sf::RenderTarget& target) {
	target.draw(bg);
	target.draw(ground);
	for (auto& pair : spawnPointColliders) {
		pair.second.draw(target);
	}
}
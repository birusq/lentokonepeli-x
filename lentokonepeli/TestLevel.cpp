#include "TestLevel.h"
#include <iostream>
#include "Console.h"
#include "Globals.h"
#include "FileLoader.h"
#include "Master.h"

TestLevel::TestLevel() {
	if(std::shared_ptr<sf::Texture> tex = master->fileLoader.getTexture("bg.png")) {
		bg.setTexture(*tex);
		tex->setSmooth(true);
	}

	bg.setScale(sf::Vector2f(0.5F, 0.5F));
	bg.setPosition(sf::Vector2f(0,0));

	sf::FloatRect bgRect = bg.getGlobalBounds();

	height = bgRect.height;
	width = bgRect.width;

	center = sf::Vector2f(bgRect.width/2.0F, bgRect.height/2.0F);

	spawnPoints[Team::RED_TEAM] = sf::Vector2f(120, bgRect.height - 20);
	spawnPoints[Team::BLUE_TEAM] = sf::Vector2f(bgRect.width - 120, bgRect.height - 20);

	sf::RectangleShape spawnBox(sf::Vector2f(30, 20));
	spawnBox.setOrigin(spawnBox.getSize()/2.0F);
	spawnBox.setOutlineThickness(0.2F);
	spawnBox.setFillColor(sf::Color::Transparent);
	Collider<sf::RectangleShape> spawnBoxCollider;

	// Temporarily we use shape as hitbox and render it, normally hitboxes and visuals are seperated
	spawnBoxCollider.hitbox = spawnBox;
	spawnBoxCollider.hitboxDisabled = false;

	spawnPointColliders[Team::RED_TEAM] = spawnBoxCollider;
	spawnPointColliders[Team::RED_TEAM].hitbox.setPosition(spawnPoints[Team::RED_TEAM]);
	spawnPointColliders[Team::RED_TEAM].hitbox.setOutlineColor(palette::red);

	spawnPointColliders[Team::BLUE_TEAM] = spawnBoxCollider;
	spawnPointColliders[Team::BLUE_TEAM].hitbox.setPosition(spawnPoints[Team::BLUE_TEAM]);
	spawnPointColliders[Team::BLUE_TEAM].hitbox.setOutlineColor(palette::blue);
	
	sf::RectangleShape perimiterWallSide(sf::Vector2f(10, height));
	sf::ConvexShape perimiterWallSideConvex(4);
	for(int i = 0; i < 4; i++) {
		perimiterWallSideConvex.setPoint(i, perimiterWallSide.getPoint(i));
	}

	//TODO: perimeter walls
}

void TestLevel::draw(sf::RenderTarget& target) {
	target.draw(bg);
	for (auto& pair : spawnPointColliders) {
		pair.second.draw(target);
	}
}
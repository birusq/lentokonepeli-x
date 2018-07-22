#include "TestLevel.h"
#include <iostream>
#include "Console.h"
#include "Globals.h"
#include "FileHandler.h"
#include "Master.h"

TestLevel::TestLevel() {
	if(std::shared_ptr<sf::Texture> tex = master->fileHandler.getTexture("bg.png")) {
		bg.setTexture(*tex);
		tex->setSmooth(true);
	}

	bg.setScale(sf::Vector2f(0.5F, 0.5F));
	bg.setPosition(sf::Vector2f(5,5));

	sf::FloatRect bgRect = bg.getGlobalBounds();

	borderWidth = 5.0F;

	width = bgRect.width + borderWidth * 2.0F;
	height = bgRect.height + borderWidth * 2.0F;

	center = sf::Vector2f(width/2.0F, height/2.0F);

	spawnPoints[Team::RED_TEAM] = sf::Vector2f(borderWidth + 120, height - 20 - borderWidth);
	spawnPoints[Team::BLUE_TEAM] = sf::Vector2f(width - 120 - borderWidth, height - 20 - borderWidth);

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

	mmBG = sf::RectangleShape(sf::Vector2f(width, height));
	auto grey = palette::strongGrey;
	grey.a = 100;
	mmBG.setFillColor(grey);
	mmBG.setOutlineColor(sf::Color::Black);
	mmBG.setOutlineThickness(5.0F);
}

void TestLevel::draw(sf::RenderTarget& target, bool minimapPass) {
	if(minimapPass) {
		target.draw(mmBG);
		return;
	}

	target.draw(bg);
	for (auto& pair : spawnPointColliders) {
		pair.second.draw(target);
	}
}
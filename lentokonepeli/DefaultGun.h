#pragma once
#include "Weapon.h"
#include <SFML/System.hpp>

class DefaultGun : public Weapon {
public:
	DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_);
	sf::Clock clock;

	float fireInterval = 0.2F;

	void draw(sf::RenderTarget& target) override;

	int shoot(sf::Uint16 bulletId, bool myShip) override;
	int shoot() override;
};
#pragma once
#include "Weapon.h"
#include <SFML/System.hpp>

class DefaultGun : public Weapon {
public:
	DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_);
	sf::Clock clock;

	float fireInterval = 0.2F;

	void draw(sf::RenderTarget& target) override;

	bool shoot(sf::Vector2f pos, float rot, sf::Uint16 bulletId) override;
	bool shoot() override;
};
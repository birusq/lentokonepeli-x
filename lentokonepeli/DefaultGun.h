#pragma once
#include "Weapon.h"
#include "CountdownTimer.h"

class DefaultGun : public Weapon {
public:
	DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_);
	sf::Clock clock;

	float fireInterval = 0.2F;

	CountdownTimer muzzleFlashTimer;
	int lastMuzzleFlashTexIndex;
	sf::Sprite muzzleFlashSprite;

	void draw(sf::RenderTarget& target) override;

	int shoot(sf::Uint16 bulletId, bool myShip) override;
	int shoot() override;
};
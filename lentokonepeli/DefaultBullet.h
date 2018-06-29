#pragma once
#include "Bullet.h"

class DefaultBullet : public Bullet {
public:
	DefaultBullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_);

	float radius = 0.8F;
	sf::CircleShape model;

	void updateHitbox() override;
	void onCollision() override;

	void launch(sf::Vector2f pos, float direction) override;

	void draw(sf::RenderTarget& target) override;
};

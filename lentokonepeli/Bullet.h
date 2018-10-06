#pragma once
#include "PhysicsTransformable.h"
#include "Collider.h"

class GOManager;

class Bullet : public PhysicsTransformable, public Collider<sf::CircleShape> {
public:
	Bullet() {}
	Bullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_);

	GOManager* goManager;

	sf::Uint16 damage = 10;
	float speed = 140.0F;
	float lifeTime = 3.0F;
	sf::Clock lifeTimeCounter;

	sf::Uint8 clientId;
	sf::Uint16 bulletId;

	virtual void updateHitbox() = 0;
	virtual void onCollision() = 0;

	virtual void launch(sf::Vector2f pos, float direction) = 0;

	virtual void draw(sf::RenderTarget& target) = 0;
};
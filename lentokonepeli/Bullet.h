#pragma once
#include "PhysicsTransformable.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "Collider.h"

class GOManager;

class Bullet : public PhysicsTransformable, public Collider<sf::RectangleShape> {
public:
	Bullet() {}
	Bullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_);

	GOManager* goManager;

	sf::Uint16 damage = 1;
	float radius = 1.0F;
	float speed = 150.0F;
	float lifeTime = 3.0F;
	sf::Clock lifeTimeCounter;
	sf::CircleShape model;

	sf::Vector2f startPos;

	sf::Uint8 clientId;
	sf::Uint16 bulletId;

	void updateHitbox() override;
	void onCollision() override;

	void launch(sf::Vector2f pos, float direction);

	void draw(sf::RenderTarget& target);
};
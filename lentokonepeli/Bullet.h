#pragma once
#include "PhysicsTransformable.h"
#include "CircleCollider.h"
#include <SFML/Graphics.hpp>

class Bullet : public PhysicsTransformable, public CircleCollider {
public:
	Bullet();

	const float radius = 1.0F;
	const float speed = 10.0F;
	sf::CircleShape model;

	bool disabled = true;

	void shoot(sf::Vector2f pos, float direction);

	void draw(sf::RenderTarget& target);
};
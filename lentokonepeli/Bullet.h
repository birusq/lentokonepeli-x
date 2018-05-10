#pragma once
#include "PhysicsTransformable.h"
#include "CircleCollider.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

class GOManager;

class Bullet : public PhysicsTransformable, public CircleCollider {
public:
	Bullet() {}
	Bullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_);

	GOManager* goManager;

	float radius = 1.0F;
	float speed = 150.0F;
	float lifeTime = 3.0F;
	sf::Clock lifeTimeCounter;
	sf::CircleShape model;

	sf::Vector2f startPos;

	sf::Uint8 clientId;
	sf::Uint16 bulletId;

	void launch(sf::Vector2f pos, float direction);

	void draw(sf::RenderTarget& target);
};
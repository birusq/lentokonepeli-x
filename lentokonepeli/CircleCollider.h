#pragma once
#include "Collider.h"
#include <SFML\Graphics.hpp>

class CircleCollider : Collider {
public:
	sf::CircleShape hitbox;
	void setTransform(sf::Vector2f pos, float rot) { hitbox.setPosition(pos); hitbox.setRotation(rot); }
	bool collision(Collider& other) {/*TODO*/ return false; }
};
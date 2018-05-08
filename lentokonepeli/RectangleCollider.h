#pragma once
#include "Collider.h"
#include <SFML\Graphics.hpp>

class RectangleCollider : Collider {
public:
	sf::RectangleShape hitbox;
	bool collision(Collider& other) {/*TODO*/ return false; }
};
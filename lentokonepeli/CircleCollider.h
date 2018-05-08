#pragma once
#include "Collider.h"
#include <SFML\Graphics.hpp>

class CircleCollider : Collider {
public:
	sf::CircleShape hitbox;
};
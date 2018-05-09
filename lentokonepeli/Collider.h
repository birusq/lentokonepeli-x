#pragma once

class Collider {
	virtual bool collision(Collider& other) = 0;
	virtual void setTransform(sf::Vector2f pos, float rot) = 0;
};
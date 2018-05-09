#include "Bullet.h"

Bullet::Bullet() {
	model.setRadius(radius);
	model.setOrigin(sf::Vector2f(radius, radius));
	model.setFillColor(sf::Color::Black);
	disabled = true;
}

void Bullet::shoot(sf::Vector2f pos, float direction) {
	setRotation(direction);
	constantVelocity = true;
	velocity = getRotationVector() * speed;
	disabled = false;
}

void Bullet::draw(sf::RenderTarget& target) {
	if (disabled == false) {

	}
}
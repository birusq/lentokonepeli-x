#include "Bullet.h"
#include "Console.h"
#include "GOManager.h"
#include "Globals.h"

Bullet::Bullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_) {
	goManager = goManager_;
	pTransId = pTransId_;
	clientId = clientId_;
	bulletId = bulletId_;
	model.setRadius(radius);
	model.setOrigin(sf::Vector2f(radius, radius));
	model.setFillColor(sf::Color::Black);
}

void Bullet::launch(sf::Vector2f pos, float direction) {
	setRotation(direction);
	setPosition(pos);
	startPos = pos;
	constantVelocity = true;
	velocity = getRotationVector() * speed;
	lifeTimeCounter.restart();
}

void Bullet::draw(sf::RenderTarget& target) {
	model.setPosition(getPosition());
	target.draw(model);
}
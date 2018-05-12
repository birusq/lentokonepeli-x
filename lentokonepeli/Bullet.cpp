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

	hitbox.setSize(sf::Vector2f(radius*2.0F, radius*2.0F));
	hitbox.setOrigin(sf::Vector2f(radius, radius));
}

void Bullet::launch(sf::Vector2f pos, float direction) {
	hitboxDisabled = false;
	setRotation(direction);
	setPosition(pos);
	constantVelocity = true;
	velocity = getRotationVector() * speed;
	lifeTimeCounter.restart();
}

void Bullet::draw(sf::RenderTarget& target) {
	if (lifeTimeCounter.getElapsedTime().asSeconds() > lifeTime) {
		goManager->removeBullet(clientId, bulletId);
	}
	model.setPosition(getPosition());
	target.draw(model);
}

void Bullet::updateHitbox() {
	if (lifeTimeCounter.getElapsedTime().asSeconds() > lifeTime) {
		goManager->removeBullet(clientId, bulletId);
	}
	hitbox.setPosition(getPosition());
	hitbox.setRotation(getRotation());
}

void Bullet::onCollision() {
	goManager->removeBullet(clientId, bulletId);
}
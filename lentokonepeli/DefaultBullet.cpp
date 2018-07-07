#include "DefaultBullet.h"
#include "GOManager.h"
#include "Console.h"


DefaultBullet::DefaultBullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_): Bullet(goManager_, pTransId_, clientId_, bulletId_) {
	model.setRadius(radius);
	model.setOrigin(sf::Vector2f(radius, radius));
	model.setFillColor(sf::Color::Black);

	hitbox.setRadius(radius);
	hitbox.setOrigin(sf::Vector2f(radius, radius));
	hitbox.setPointCount(5);

	speed = 200;

	lifeTime = 2.0F;
}

void DefaultBullet::updateHitbox() {
	if (lifeTimeCounter.getElapsedTime().asSeconds() > lifeTime) {
		goManager->removeBullet(clientId, bulletId);
	}
	hitbox.setPosition(getPosition());
	hitbox.setRotation(getRotation());
}

void DefaultBullet::onCollision() {
	goManager->removeBullet(clientId, bulletId);
}

void DefaultBullet::launch(sf::Vector2f pos, float direction) {
	hitboxDisabled = false;
	setRotation(direction);
	setPosition(pos);
	constantVelocity = true;
	velocity = getRotationVector() * speed;
	lifeTimeCounter.restart();
}

void DefaultBullet::draw(sf::RenderTarget & target) {
	if (lifeTimeCounter.getElapsedTime().asSeconds() > lifeTime) {
		goManager->removeBullet(clientId, bulletId);
	}
	model.setPosition(getPosition());
	target.draw(model);
}



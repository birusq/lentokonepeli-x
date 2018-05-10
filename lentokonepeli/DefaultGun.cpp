#include "DefaultGun.h"
#include "GOManager.h"
#include "Console.h"

DefaultGun::DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_) : Weapon(goManager_, ownerId_) {}

void DefaultGun::draw(sf::RenderTarget& target) {
	/*sf::RectangleShape cs;
	cs.setSize(sf::Vector2f(3.0F, 2.0F));
	cs.setOrigin(cs.getSize() / 2.0F);
	cs.setFillColor(sf::Color::Red);
	cs.setPosition(getPosition());
	cs.setRotation(getRotation());
	target.draw(cs);*/
}

bool DefaultGun::shoot() {
	return shoot(getPosition(), getRotation(), goManager->getNewBulletId(ownerId));
}

bool DefaultGun::shoot(sf::Vector2f pos, float rot, sf::Uint16 bulletId) {
	if (clock.getElapsedTime() > sf::seconds(fireInterval)) {
		clock.restart();
		Bullet* bullet = goManager->createBullet(ownerId, bulletId);
		bullet->launch(pos, rot);
		goManager->addToPhysics(bullet);
		return true;
	}
	else {
		return false;
	}
}
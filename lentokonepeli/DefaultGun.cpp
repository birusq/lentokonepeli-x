#include "DefaultGun.h"
#include "GOManager.h"
#include "Console.h"
#include "Bullet.h"
#include "Master.h"
#include "DefaultBullet.h"

DefaultGun::DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_) : Weapon(goManager_, ownerId_) {
	localTransform.transformPoint(0, -5);
}

void DefaultGun::draw(sf::RenderTarget& target) {
	sf::RectangleShape cs;
	cs.setSize(sf::Vector2f(3.0F, 2.0F));
	cs.setOrigin(cs.getSize() / 2.0F);
	cs.setFillColor(sf::Color::Red);
	cs.setPosition(getPosition());
	cs.setRotation(getRotation());
	target.draw(cs);
}

int DefaultGun::shoot() {
	return shoot(goManager->getNewBulletId(ownerId), true);
}

int DefaultGun::shoot(sf::Uint16 bulletId, bool myShip) {
	if (clock.getElapsedTime() > sf::seconds(fireInterval) || (myShip == false && clock.getElapsedTime() > sf::seconds(fireInterval * 0.7F))) { // make timing less harsh for others because of latency
		clock.restart();
		Bullet* bullet = new DefaultBullet(goManager, goManager->getUnusedPTransId(), ownerId, bulletId);
		bullet->launch(getPosition(), getRotation());
		goManager->addBullet(bullet);
		master->soundPlayer.playSound(getPosition(), "shoot");
		return bulletId;
	}
	else {
		return -1;
	}
}
#include "DefaultGun.h"
#include "GOManager.h"
#include "Console.h"
#include "Bullet.h"
#include "Master.h"
#include "DefaultBullet.h"

DefaultGun::DefaultGun(GOManager* goManager_, sf::Uint8 ownerId_) : Weapon(goManager_, ownerId_) {
	localTransform.transformPoint(0, -5);

	if(!inServer) {
		std::shared_ptr<sf::Texture> tex = master->fileHandler.getTexture("muzzleFlash_atlas.png");
		tex->setSmooth(true);
		muzzleFlashSprite.setTexture(*tex);
		muzzleFlashSprite.setScale(0.6F,0.6F);
		muzzleFlashSprite.setOrigin(0.0F, 16.0F);
		muzzleFlashSprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
	}
}

void DefaultGun::draw(sf::RenderTarget& target) {
	/*sf::RectangleShape cs;
	cs.setSize(sf::Vector2f(3.0F, 2.0F));
	cs.setOrigin(cs.getSize() / 2.0F);
	cs.setFillColor(sf::Color::Red);
	cs.setPosition(getPosition());
	cs.setRotation(getRotation());
	target.draw(cs);*/

	if(!muzzleFlashTimer.isDone()) {
		muzzleFlashSprite.setPosition(getPosition());
		muzzleFlashSprite.setRotation(getRotation() - 90.0F);
		muzzleFlashSprite.setColor(sf::Color(255, 255, 255, sf::Uint8(muzzleFlashTimer.getTimeRemainingRatio() * 255.0F)));
		target.draw(muzzleFlashSprite);
	}
}

int DefaultGun::shoot() {
	return shoot(goManager->getNewBulletId(ownerId), true);
}

int DefaultGun::shoot(sf::Uint16 bulletId, bool myShip) {
	if (shipFullyAlive && (clock.getElapsedTime() > sf::seconds(fireInterval) || (myShip == false && clock.getElapsedTime() > sf::seconds(fireInterval * 0.4F)))) { // make timing less harsh for others because of latency
		clock.restart();
		Bullet* bullet = new DefaultBullet(goManager, goManager->getUnusedPTransId(), ownerId, bulletId);
		bullet->launch(getPosition() + g::getRotationVector(getRotation()) * bullet->hitbox.getRadius(), getRotation());
		goManager->addBullet(bullet);
		
		muzzleFlashTimer.start(0.08F);
		muzzleFlashSprite.setTextureRect(sf::IntRect(thor::random(0,3) * 32, 0, 32, 32));
		
		return bulletId;
	}
	else {
		return -1;
	}
}
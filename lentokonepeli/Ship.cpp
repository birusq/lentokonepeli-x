#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"
#include "Weapon.h"
#include "Master.h"
#include "PacketHelper.h"

Ship::Ship(sf::Uint32 pTransId_, User* owner_, Team::Id teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	pTransId = pTransId_;
	gravity = false;
	drag = 0.01F;

	setPosition(200, 100);

	float width = 3.0F;
	float height = 10.0f;

	hitbox.setSize(sf::Vector2f(width, height));
	hitbox.setOrigin(width / 2.0f, height / 2.0F);

	shipBody.setFillColor(sf::Color::Black);
	shipBody.setSize(sf::Vector2f(width, height));
	shipBody.setOrigin(width / 2.0f, height / 2.0F);
	
	exhaust.setFillColor(sf::Color::Blue);
	exhaust.setSize(sf::Vector2f(1.6F, 2.0F));
	exhaust.setOrigin(0.8F, -height / 2.0F);

	usernameLabel.setFont(g::font);
	usernameLabel.setString(owner->username.C_String());
	usernameLabel.setCharacterSize(40);
	usernameLabel.setScale(sf::Vector2f(0.065F, 0.065F));
	usernameLabel.setOrigin(usernameLabel.getLocalBounds().width/2.0F, usernameLabel.getLocalBounds().height);
	usernameLabel.setFillColor(sf::Color::Black);
	
	healthBar = sf::RectangleShape(sf::Vector2f(hbMaxLength, 1.6F));

	healthBarBG = sf::RectangleShape(sf::Vector2f(healthBar.getSize().x + hbBorderSize * 2.0F, healthBar.getSize().y + hbBorderSize *  2.0F));
	healthBarBG.setFillColor(palette::strongGrey);

	assignTeam(teamId);
}

void Ship::assignTeam(Team::Id teamId_) {
	teamId = teamId_;
	if (teamId == Team::RED_TEAM) {
		healthBar.setFillColor(palette::red);
	}
	else if (teamId == Team::BLUE_TEAM) {
		healthBar.setFillColor(palette::blue);
	}
}

void Ship::draw(sf::RenderTarget& target) {
	if (localPlayer) {
		master->soundPlayer.updateListenerPos(getPosition());
	}
	
	if (isDead() == false) {

		if (dmgTimer.getElapsedTime().asSeconds() > dmgTime) {
			shipBody.setFillColor(sf::Color::Black);
		}

		weapon->setPosition(getRotationVector() * 5.0F + getPosition());
		weapon->setRotation(getRotation());

		//weapon->draw(target);

		shipBody.setPosition(getPosition());
		shipBody.setRotation(getRotation());
		target.draw(shipBody);

		if (throttle) {
			master->soundPlayer.playThrottle(getPosition(), owner->clientId);
			exhaust.setPosition(getPosition());
			exhaust.setRotation(getRotation());
			target.draw(exhaust);
		}
		else {
			master->soundPlayer.stopThrottle(owner->clientId);
		}

		healthBar.setPosition(getPosition().x - hbMaxLength / 2.0F, getPosition().y - 8.0F);
		healthBarBG.setPosition(healthBar.getPosition().x - hbBorderSize, healthBar.getPosition().y - hbBorderSize);

		usernameLabel.setPosition(getPosition().x, getPosition().y - 10.0F);

		target.draw(healthBarBG);
		target.draw(healthBar);
		target.draw(usernameLabel);
	}
	else {
		master->soundPlayer.stopThrottle(owner->clientId);
	}
}

void Ship::updateHitbox() {
	if (isDead())
		return;

	if (dmgTimer.getElapsedTime().asSeconds() > dmgTime) {
		hitboxDisabled = false;
	}

	hitbox.setRotation(getRotation());
	hitbox.setPosition(getPosition());
}

void Ship::setWeaponTrans(sf::Vector2f pos, float rot) {
	setRotation(rot);
	weapon->setRotation(rot);
	weapon->setPosition(getRotationVector() * 5.0F + pos);
}

void Ship::onCollision() {
	hitboxDisabled = true;
}

void Ship::respawn() {
	setHealthToFull();
	hitboxDisabled = false;

	healthBar.setSize(sf::Vector2f(health / maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::takeDmg(int dmg, DamageType dmgType) {

	//maybe react differently to different damage types

	console::stream << owner->username.C_String() << " took " << dmg << " damage";
	console::dlogStream();
	shipBody.setFillColor(sf::Color::Red);
	health -= dmg;
	if (health <= 0.0F)
		onDeath();

	dmgTimer.restart();
	master->soundPlayer.playSound(getPosition(), "hurt");
	healthBar.setSize(sf::Vector2f((float)health / (float)maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::restoreHealth(int heal) {
	console::stream << owner->username.C_String() << " healed " << heal << " health";
	console::dlogStream();
	health += heal;
	if (health > maxHealth) {
		health = maxHealth;
	}

	healthBar.setSize(sf::Vector2f((float)health / (float)maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::onDeath() {
	hitboxDisabled = true;
	timeSinceDeath.restart();
}
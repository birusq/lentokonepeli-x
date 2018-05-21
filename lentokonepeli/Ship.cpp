#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"
#include "Weapon.h"
#include "Master.h"

Ship::Ship(sf::Uint32 pTransId_, User* owner_, TeamId teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	pTransId = pTransId_;
	gravity = false;
	drag = 0.01F;

	setPosition(200, 100);

	float width = 3.0F;
	float height = 10.0f;

	hitbox.setSize(sf::Vector2f(width, height));
	hitbox.setOrigin(width / 2.0f, height / 2.0F);

	rectangle.setFillColor(sf::Color::Black);
	rectangle.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	
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

void Ship::assignTeam(TeamId teamId_) {
	teamId = teamId_;
	if (teamId == RED_TEAM) {
		healthBar.setFillColor(palette::red);
	}
	else if (teamId == BLUE_TEAM) {
		healthBar.setFillColor(palette::blue);
	}
}

void Ship::draw(sf::RenderTarget& target) {
	if (localPlayer) {
		master->soundPlayer.updateListenerPos(getPosition());
	}
	
	if (isDead() == false) {

		if (dmgTimer.getElapsedTime().asSeconds() > dmgTime) {
			rectangle.setFillColor(sf::Color::Black);
		}

		rectangle.setRotation(getRotation());
		rectangle.setPosition(getPosition());

		weapon->setPosition(getRotationVector() * 5.0F + getPosition());
		weapon->setRotation(getRotation());

		weapon->draw(target);

		target.draw(rectangle);

		healthBar.setPosition(getPosition().x - hbMaxLength / 2.0F, getPosition().y - 8.0F);
		healthBarBG.setPosition(healthBar.getPosition().x - hbBorderSize, healthBar.getPosition().y - hbBorderSize);

		usernameLabel.setPosition(getPosition().x, getPosition().y - 10.0F);

		target.draw(healthBarBG);
		target.draw(healthBar);
		target.draw(usernameLabel);
		

		if (throttle) {
			master->soundPlayer.playThrottle(getPosition(), owner->clientId);
		}
		else {
			master->soundPlayer.stopThrottle(owner->clientId);
		}
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
	dmgTimer.restart();
	rectangle.setFillColor(sf::Color::Red);
}

void Ship::respawn() {
	setHealthToFull();
	hitboxDisabled = false;

	healthBar.setSize(sf::Vector2f(health / maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::takeDmg(float dmg) {
	console::dlog(std::string(owner->username.C_String()) + std::string(" took damage"));
	rectangle.setFillColor(sf::Color::Red);
	health -= dmg;
	if (health <= 0.0F)
		onDeath();

	healthBar.setSize(sf::Vector2f(health / maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::restoreHealth(float heal) {
	health += heal;
	if (health > maxHealth) {
		health = maxHealth;
	}

	healthBar.setSize(sf::Vector2f(health / maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::onDeath() {
	hitboxDisabled = true;
	timeSinceDeath.restart();
}
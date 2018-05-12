#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"

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
	usernameLabel.setCharacterSize(16);
	usernameLabel.setOrigin(usernameLabel.getGlobalBounds().width/2.0F, usernameLabel.getGlobalBounds().height);
	usernameLabel.setFillColor(sf::Color::Black);

	
	healthBar = sf::RectangleShape(sf::Vector2f(hbMaxLength, 10));
	healthBar.setOrigin(sf::Vector2f(0, 10));

	healthBarBG = sf::RectangleShape(sf::Vector2f(hbMaxLength + 4, 14));
	healthBarBG.setOrigin(sf::Vector2f(2, 12));
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

		sf::View view = target.getView();
		sf::View defaultView = target.getDefaultView();
		target.setView(defaultView);

		sf::Vector2f posDiff = view.getCenter() - 0.5F * view.getSize();
		sf::Vector2f planePos = getPosition();

		float factorX = (defaultView.getSize().x / view.getSize().x);
		float factorY = (defaultView.getSize().y / view.getSize().y);
		
		sf::Vector2f planePosInUI = sf::Vector2f((planePos.x - posDiff.x) * factorX, (planePos.y - posDiff.y) * factorY);

		healthBar.setPosition(planePosInUI.x - hbMaxLength/2.0F, planePosInUI.y - 7.0F * factorY);
		healthBarBG.setPosition(healthBar.getPosition());

		target.draw(healthBarBG);
		target.draw(healthBar);

		usernameLabel.setPosition(planePosInUI.x, healthBar.getPosition().y - 18.0F);

		target.draw(usernameLabel);

		target.setView(view);
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
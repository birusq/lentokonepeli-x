#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"

Ship::Ship(sf::Uint32 pTransId_, User* owner_, TeamId teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	pTransId = pTransId_;
	gravity = false;
	drag = 0.01F;

	float width = 3.0F;
	float height = 10.0f;


	hitbox.setSize(sf::Vector2f(width, height));
	hitbox.setOrigin(width / 2.0f, height / 2.0F);

	rectangle.setFillColor(sf::Color::Black);
	rectangle.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	
	usernameLabel.setFont(g::font);
	usernameLabel.setString(owner->username.C_String());
	usernameLabel.setCharacterSize(14);
	usernameLabel.setOrigin(usernameLabel.getGlobalBounds().width/2.0F, usernameLabel.getGlobalBounds().height/2.0F);
	usernameLabel.setFillColor(sf::Color::Black);

	assignTeam(teamId);
}

void Ship::assignTeam(TeamId teamId_) {
	teamId = teamId_;
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
		
		sf::Vector2f planePosInUI = sf::Vector2f((planePos.x - posDiff.x) * (defaultView.getSize().x / view.getSize().x), (planePos.y - posDiff.y) *  (defaultView.getSize().y / view.getSize().y));
		
		usernameLabel.setPosition(planePosInUI.x, planePosInUI.y - 8 * (defaultView.getSize().x / view.getSize().x));

		target.draw(usernameLabel);

		target.setView(view);
	}
}

void Ship::updateHitbox() {
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

void Ship::takeDmg(float dmg) {
	console::dlog(std::string(owner->username.C_String()) + std::string(" took damage"));
	rectangle.setFillColor(sf::Color::Red);
	health -= dmg;
	if (health <= 0.0F)
		onDeath();
}

void Ship::restoreHealth(float heal) {
	health += heal;
	if (health > maxHealth) {
		health = maxHealth;
	}
}

void Ship::onDeath() {

}
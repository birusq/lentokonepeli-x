#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"

Ship::Ship(sf::Uint32 pTransId_, User* const owner_, TeamId teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	objType = SHIP;
	pTransId = pTransId_;
	gravity = false;
	drag = 0.01F;

	float width = 3.0F;
	float height = 10.0f;

	hitbox.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	rectangle.setFillColor(sf::Color::Black);
	
	rectangle.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	rectangle.setFillColor(sf::Color::Black);
	
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

		weapon->setPosition(getRotationVector() * 5.0F + getPosition());
		weapon->setRotation(getRotation());
		weapon->draw(target);

		rectangle.setRotation(getRotation());
		rectangle.setPosition(getPosition());
		
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

void Ship::takeDmg(float dmg) {
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
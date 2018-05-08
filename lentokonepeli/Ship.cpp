#include "Ship.h"
#include "Globals.h"

Ship::Ship(User* const owner_, TeamId teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	float width = 3.0F;
	float height = 10.0f;
	rectangle.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	rectangle.setFillColor(sf::Color::Black);
	gravity = false;
	drag = 0.01F;

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
		rectangle.setRotation(getRotation());
		rectangle.setPosition(getPosition());
		
		target.draw(rectangle);

		sf::View view = target.getView();
		sf::Vector2f viewSize = view.getSize();
		sf::Vector2f pixelSize = target.getDefaultView().getSize();

		target.setView(target.getDefaultView());

		usernameLabel.setPosition(sf::Vector2f(pixelSize.x / viewSize.x * getPosition().x , pixelSize.y / viewSize.y * (getPosition().y + 6)));
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
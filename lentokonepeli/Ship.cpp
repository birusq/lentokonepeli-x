#include "Ship.h"

Ship::Ship(User* const owner_, TeamId teamId_) : owner{ owner_ }, teamId{ teamId_ } {
	float width = 3.0F;
	float height = 10.0f;
	rectangle.setSize(sf::Vector2f(width, height));
	rectangle.setOrigin(width / 2.0f, height / 2.0F);
	rectangle.setFillColor(sf::Color::Black);
	gravity = false;
	drag = 0.01F;

	assignTeam(teamId);
}

void Ship::assignTeam(TeamId teamId_) {
	teamId = teamId_;
	if (teamId == RED_TEAM) {
		rectangle.setFillColor(sf::Color(193, 34, 34));
	}
	else if (teamId == BLUE_TEAM) {
		rectangle.setFillColor(sf::Color(0, 28, 132));
	}
}

void Ship::updateVisuals() {
	if (isDead() == false) {
		rectangle.setRotation(getRotation());
		rectangle.setPosition(getPosition());
	}
}

void Ship::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (isDead() == false) {
		target.draw(rectangle, states);
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
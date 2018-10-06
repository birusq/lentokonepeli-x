#include "GameObject.h"


GameObject::GameObject(std::string name_, sf::Vector2f size = sf::Vector2f(1, 1), sf::Vector2f pos = sf::Vector2f(0, 0), float rot = 0.0F, bool grav = false, float drag = 0.0F) {
	name = name_;
	hitbox = sf::RectangleShape(size);
	hitbox.setPosition(pos);
	hitbox.setOrigin(size / 2.0F);

	hitbox.setFillColor(sf::Color::White);

	pTrans.setPosition(pos);
	pTrans.setRotation(rot);
	pTrans.gravity = grav;
	pTrans.drag = drag;

}

void GameObject::draw(sf::RenderTarget& target) {
	hitbox.setPosition(pTrans.getPosition());
	hitbox.setRotation(pTrans.getRotation());
	target.draw(hitbox);
}
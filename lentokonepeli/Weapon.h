#pragma once
#include <SFML/Graphics.hpp>

class Ship;
class GOManager;

class Weapon : public sf::Transformable {
public:
	Weapon(GOManager* goManager_, sf::Uint8 ownerId_) : goManager{ goManager_ }, ownerId{ownerId_} {};

	GOManager* goManager;
	sf::Uint8 ownerId;

	virtual void draw(sf::RenderTarget& target) = 0;
	//Used when you own this weapon
	virtual bool shoot() = 0;
	//Used to spawn bullets for other ships
	virtual bool shoot(sf::Vector2f pos, float rot, sf::Uint16 bulletId) = 0;
};

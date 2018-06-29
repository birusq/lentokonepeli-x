#pragma once
#include <SFML/Graphics.hpp>

class Ship;
class GOManager;

class Weapon : public sf::Transformable {
public:
	Weapon(GOManager* goManager_, sf::Uint8 ownerId_) : goManager{ goManager_ }, ownerId{ ownerId_ } { };

	GOManager* goManager;
	sf::Uint8 ownerId;

	sf::Transform localTransform;

	bool shipFullyAlive = false;

	virtual void draw(sf::RenderTarget& target) = 0;
	//Used when you own this weapon (returns bulletId, -1 if could't shoot)
	virtual int shoot() = 0;
	//Used to spawn bullets for other ships (returns bulletId, -1 if could't shoot)
	virtual int shoot(sf::Uint16 bulletId, bool myShip) = 0;
};

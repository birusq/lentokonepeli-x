#pragma once

class Master;

namespace palette {
	extern sf::Color red;
	extern sf::Color blue;
	extern sf::Color strongGrey;
	extern sf::Color yellow;
}

namespace g {

	extern sf::Font font;
	extern const std::string version;

	void init();

	// get unit vector facing a direction
	sf::Vector2f getRotationVector(float rotation);

	enum ObjectType : sf::Uint8 {
		SHIP,
		BULLET
	};
}
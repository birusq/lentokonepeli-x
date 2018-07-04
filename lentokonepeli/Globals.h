#pragma once

#include <SFML/Graphics.hpp>

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

	enum ObjectType : sf::Uint8 {
		SHIP,
		BULLET
	};
}
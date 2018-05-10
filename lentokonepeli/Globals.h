#pragma once

#include <SFML/Graphics.hpp>


namespace palette {
	extern sf::Color red;
	extern sf::Color blue;
	extern sf::Color strongGrey;
}

namespace g {
	extern sf::Font font;
	void init();

	enum ObjectType : sf::Uint8 {
		SHIP,
		BULLET
	};

	float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b);
}
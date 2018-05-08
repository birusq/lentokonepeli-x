#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

namespace palette {
	extern sf::Color red;
	extern sf::Color blue;
	extern sf::Color strongGrey;
}

namespace g {
	extern sf::Font font;
	void init();
}
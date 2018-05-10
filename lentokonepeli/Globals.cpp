#include "Globals.h"

namespace palette {
	sf::Color red = sf::Color(186, 0, 0);
	sf::Color blue = sf::Color(0, 0, 186);
	sf::Color strongGrey = sf::Color(62, 65, 71);
}

namespace g {
	sf::Font font;
	
	void init() {
		font.loadFromFile("res/verdana.ttf");
	}

	float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) {
		return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	}
}
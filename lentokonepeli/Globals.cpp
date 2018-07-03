
#include "Globals.h"

namespace palette {
	sf::Color red = sf::Color(255, 58, 58);
	sf::Color blue = sf::Color(30, 101, 255);
	sf::Color strongGrey = sf::Color(62, 65, 71);
}

namespace g {

	sf::Font font;

	const std::string version = "0.6-alpha";
	
	void init() {
		font.loadFromFile("res/verdana.ttf");
	}
}
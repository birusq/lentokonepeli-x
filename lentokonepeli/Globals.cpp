
#include "Globals.h"

namespace palette {
	sf::Color red = sf::Color(255, 56, 56);
	sf::Color blue = sf::Color(55, 82, 255);
	sf::Color strongGrey = sf::Color(62, 65, 71);
	sf::Color yellow = sf::Color(252, 205, 37);
}	

namespace g {

	sf::Font font;

	// Don't use "v" prefix here
	const std::string version = "0.8-alpha";
	
	void init() {
		font.loadFromFile("res/verdana.ttf");
	}
}
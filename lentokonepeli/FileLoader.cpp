#include "FileLoader.h"
#include "Console.h"

std::shared_ptr<sf::Texture> FileLoader::getTexture(std::string imagePath) {
	if(textures.count(imagePath) == 1) {
		return textures[imagePath];
	}
	else {
		textures[imagePath] = std::make_shared<sf::Texture>();
		if((*textures[imagePath]).loadFromFile("res/images/" + imagePath)) {
			return textures[imagePath];
		}
		else {
			console::stream << "Could not find resource from path " << imagePath;
			console::dlogStream();
			textures.erase(imagePath);
			return nullptr;
		}
	}
}

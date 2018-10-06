#include "FileHandler.h"
#include "Console.h"


std::shared_ptr<sf::Texture> FileHandler::getTexture(std::string imagePath) {
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

std::shared_ptr<sf::Image> FileHandler::getImage(std::string imagePath) {
	if(images.count(imagePath) == 1) {
		return images[imagePath];
	}
	else {
		images[imagePath] = std::make_shared<sf::Image>();
		if((*images[imagePath]).loadFromFile("res/images/" + imagePath)) {
			return images[imagePath];
		}
		else {
			console::stream << "Could not find resource from path " << imagePath;
			console::dlogStream();
			images.erase(imagePath);
			return nullptr;
		}
	}
}

namespace fs = std::filesystem;

void FileHandler::saveScreenShot(sf::Image& image) {
	auto path = fs::absolute("screenshots");
	if(!fs::exists(path)) {
		fs::create_directory(path);
	}
	
	int ssIndex = 0;
	for(; ssIndex < 10000; ssIndex++) {
		std::string fileName = "ss" + std::to_string(ssIndex) + ".png";
		if(!fs::exists(path.string() + "/" + fileName)) {
			if(image.saveToFile("screenshots/" + fileName)) {
				break;
			}
			else {
				console::dlog("Couldn't save screenshot");
			}
		}
	}

	if (ssIndex == 10000)
		console::dlog("Couldn't save screenshot");
}

#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>

class FileLoader {
public:
	// Get resource from path relative to "res/images/"
	std::shared_ptr<sf::Texture> getTexture(std::string imagePath);

	// Get resource from path relative to "res/images/"
	std::shared_ptr<sf::Image> getImage(std::string imagePath);
private:
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<sf::Image>> images;
};
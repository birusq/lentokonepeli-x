#pragma once


class FileHandler {
public:
	// Get resource from path relative to "res/images/"
	std::shared_ptr<sf::Texture> getTexture(std::string imagePath);

	// Get resource from path relative to "res/images/"
	std::shared_ptr<sf::Image> getImage(std::string imagePath);

	void saveScreenShot(sf::Image& image);

private:
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<sf::Image>> images;
};
#pragma once
#include "SFML\Graphics.hpp"
#include "FpsCounter.h"
#include "Closeable.h"

class Master;
class GUI;

class MainMenu : public FpsCounter, public Closeable {
public:
	MainMenu(Master* master_);
	void loop();
	void render(sf::RenderWindow& w);
private:
	GUI* gui;
	Master* master;
	void quit();
};
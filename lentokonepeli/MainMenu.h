#pragma once
#include "FpsCounter.h"
#include "Closeable.h"

class Master;
class GUI;

class MainMenu : public FpsCounter, public Closeable {
public:
	MainMenu();
	void loop();
	void render(sf::RenderWindow& w, float dt);
private:
	GUI* gui;
	void quit();
};
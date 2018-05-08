#include "SFML/Window.hpp"
#include "SimpleIni.h"
#include "Settings.h"
#include <iostream>
#include <fstream>
#include <Thor\Input.hpp>

//Init
Settings::Settings() {

	ini.LoadFile("config.ini");

	if (ini.IsEmpty()) {
		std::ofstream file("config.ini");
		ini.LoadFile("config.ini");
	}

	width = IntSetting(&ini, "settings", "width", 800);

	style = sf::Style::Titlebar | sf::Style::Close;
	height = IntSetting(&ini, "settings", "height", 600);
	fsWidth = IntSetting(&ini, "settings", "fsWidth", 1920);
	fsHeight = IntSetting(&ini, "settings", "fsHeight", 1080);

	fullscreen = BoolSetting(&ini, "settings", "fullscreen", false);
	if (fullscreen.getValue()) {
		style = sf::Style::Fullscreen;
	}

	borderless = BoolSetting(&ini, "settings", "borderless", false);
	if (borderless.getValue()) {
		style = sf::Style::None;
	}

	framerateLimit = IntSetting(&ini, "settings", "framerateLimit", 300);
	vsync = BoolSetting(&ini, "settings", "vsync", false);

	showFps = BoolSetting(&ini, "settings", "showFps", true);
	showPing = BoolSetting(&ini, "settings", "showPing", true);

	antialiasingLevel = IntSetting(&ini, "graphics", "antialiasingLevel", 4);

	username = StringSetting(&ini, "user", "username", "*");

	moveForwardKey = KeySetting(&ini, "keybinds", "moveForward", sf::Keyboard::Up);
	turnLeftKey = KeySetting(&ini, "keybinds", "turnLeft", sf::Keyboard::Left);
	turnRightKey = KeySetting(&ini, "keybinds", "turnRight", sf::Keyboard::Right);
	shootKey = KeySetting(&ini, "keybinds", "turnRight", sf::Keyboard::Space);
}
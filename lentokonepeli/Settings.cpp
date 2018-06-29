#include "SFML/Window.hpp"
#include "SimpleIni.h"
#include "Settings.h"
#include <iostream>
#include <fstream>
#include <Thor\Input.hpp>
#include "Console.h"

//Init
Settings::Settings() {

	ini.LoadFile("config.ini");

	if (ini.IsEmpty()) {
		std::ofstream file("config.ini");
		ini.LoadFile("config.ini");
	}

	width = IntSetting(&ini, "graphics", "width", 800);

	style = sf::Style::Titlebar | sf::Style::Close;
	height = IntSetting(&ini, "graphics", "height", 600);
	fsWidth = IntSetting(&ini, "graphics", "fsWidth", -1);
	fsHeight = IntSetting(&ini, "graphics", "fsHeight", -1);
	if(fsWidth == -1 || fsHeight == -1) {
		fsWidth.setValue(sf::VideoMode::getDesktopMode().width);
		fsHeight.setValue(sf::VideoMode::getDesktopMode().height);
	}

	fullscreen = BoolSetting(&ini, "graphics", "fullscreen", false);
	if (fullscreen.getValue()) {
		style = sf::Style::Fullscreen;
	}

	borderless = BoolSetting(&ini, "graphics", "borderless", true);
	
	if (fullscreen.getValue() && borderless.getValue()) {
		style = sf::Style::None;
	}

	framerateLimit = IntSetting(&ini, "graphics", "framerateLimit", 300);
	vsync = BoolSetting(&ini, "graphics", "vsync", false);
	antialiasingLevel = IntSetting(&ini, "graphics", "antialiasingLevel", 4);

	guiScalePercent = IntSetting(&ini, "gui", "guiScalePercent", -1);
	showFps = BoolSetting(&ini, "gui", "showFps", true);
	showPing = BoolSetting(&ini, "gui", "showPing", true);

	masterVolumePercent = IntSetting(&ini, "audio", "masterVolumePercent", 20);

	username = StringSetting(&ini, "user", "username", "*", 20);

	moveForwardKey = KeySetting(&ini, "keybinds", "moveForward", sf::Keyboard::Up);
	turnLeftKey = KeySetting(&ini, "keybinds", "turnLeft", sf::Keyboard::Left);
	turnRightKey = KeySetting(&ini, "keybinds", "turnRight", sf::Keyboard::Right);
	shootKey = KeySetting(&ini, "keybinds", "shoot", sf::Keyboard::Space);

	abilityForwardKey = KeySetting(&ini, "keybinds", "abilityForward", sf::Keyboard::W);
	abilityLeftKey = KeySetting(&ini, "keybinds", "abilityLeft", sf::Keyboard::A);
	abilityBackwardKey = KeySetting(&ini, "keybinds", "abilityBackward", sf::Keyboard::S);
	abilityRightKey = KeySetting(&ini, "keybinds", "abilityRight", sf::Keyboard::D);

	inGameMenuKey = KeySetting(&ini, "keybinds", "inGameMenu", sf::Keyboard::Escape);
	scoreBoardKey = KeySetting(&ini, "keybinds", "scoreBoard", sf::Keyboard::Tab);

	console::dlog("asdfasdfjaslkdjlaösdfökladshfjk");
}

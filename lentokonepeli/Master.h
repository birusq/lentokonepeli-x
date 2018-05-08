#pragma once

#include "RakNet/WindowsIncludes.h"
#include "Raknet\RakPeerInterface.h"
#include "Settings.h"
#include "FpsCounter.h"
#include "Closeable.h"
#include "Game.h"
#include "SFML\Graphics.hpp"
#include "MainMenu.h"
#include "GUI.h"
#include "Console.h"

enum class WindowState {
	MainMenu,
	GameServer,
	GameClient
};

class Master {
public:
	Master();

	void Master::createWindow(WindowState state);

	int loop();

	WindowState currentWindowState = WindowState::MainMenu;

	Closeable* currentCloseable;

	void quit();

	GUI gui;

	Settings settings;
	sf::RenderWindow window;

	FpsCounter* fpsCounter;

	void launchMainMenu();
	void launchHost();
	void launchClient(std::string hostIp_);
	std::string hostIp;

	sf::View view;
private:
	bool run = true;
};
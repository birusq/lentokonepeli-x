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
#include "SoundPlayer.h"

enum class WindowState {
	MainMenu,
	GameServer,
	GameClient
};

class Master {
public:
	Master();

	int loop();

	void quit();

	Settings settings;

	GUI gui;
	SoundPlayer soundPlayer;

	sf::RenderWindow window;

	void launchMainMenu();
	void launchHost();
	void launchClient(std::string hostIp_);
	
	FpsCounter* fpsCounter;
	Closeable* currentCloseable;

private:
	WindowState currentWindowState = WindowState::MainMenu;
	void createWindow(WindowState state);

	std::string hostIp;

	bool run = true;
};

extern Master* master;
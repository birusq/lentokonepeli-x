#pragma once

#include "Settings.h"
#include "Game.h"
#include "MainMenu.h"
#include "GUI.h"
#include "Console.h"
#include "SoundPlayer.h"
#include "FileHandler.h"

class FpsCounter;
class Closeable;

enum class WindowState {
	MainMenu,
	GameServer,
	GameClient
};

class Master {
public:
	Master() {};

	void init();

	int loop();

	void quit();

	Settings settings;

	GUI gui;
	SoundPlayer soundPlayer;
	FileHandler fileHandler;

	sf::RenderWindow window;

	sf::Vector2f defaultViewSize;

	void launchMainMenu();
	void launchHost();
	void launchClient(RakNet::SystemAddress hostAddress_);
	
	RakNet::SystemAddress hostAddress;

	FpsCounter* fpsCounter;
	Closeable* currentCloseable;

private:
	WindowState currentWindowState = WindowState::MainMenu;
	void createWindow(WindowState state);

	bool run = true;
};

extern Master* master;
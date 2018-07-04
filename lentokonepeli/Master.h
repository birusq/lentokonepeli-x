#pragma once

#include "RakNet/WindowsIncludes.h"
#include "Raknet\RakPeerInterface.h"
#include "Settings.h"
#include "Game.h"
#include "SFML\Graphics.hpp"
#include "MainMenu.h"
#include "GUI.h"
#include "Console.h"
#include "SoundPlayer.h"
#include "FileLoader.h"

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
	FileLoader fileLoader;

	sf::RenderWindow window;

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
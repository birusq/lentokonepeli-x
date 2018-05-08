#include "Master.h"
#include "MainMenu.h"
#include "ClientGame.h"
#include "ServerGame.h"

#define VERSION_NUMBER 1;

Master::Master() {
	console::clearLogFile();

	gui.init(this);

	launchMainMenu();
}

void Master::createWindow(WindowState state) {

	sf::ContextSettings cs;
	cs.antialiasingLevel = settings.antialiasingLevel;

	int height = settings.height;
	int width = settings.width;

	if (settings.fullscreen == true && state != WindowState::GameServer) {
		height = settings.fsHeight;
		width = settings.fsWidth;
	}

	bool windowWasOpen = window.isOpen();
	sf::Vector2i oldPos = window.getPosition();

	if (state == WindowState::GameClient)
		window.create(sf::VideoMode(width, height), "lentsikat - Client", settings.style, cs);
	else if (state == WindowState::GameServer)
		window.create(sf::VideoMode(width, height), "lentsikat - Server", sf::Style::Titlebar | sf::Style::Close, cs);
	else if (state == WindowState::MainMenu)
		window.create(sf::VideoMode(width, height), "lentsikat", sf::Style::Titlebar | sf::Style::Close, cs);

	if (windowWasOpen)
		window.setPosition(oldPos);

	if (settings.vsync == false && settings.framerateLimit != 0 && state != WindowState::GameServer) {
		window.setFramerateLimit(settings.framerateLimit);
	}

	if (state != WindowState::GameServer)
		window.setVerticalSyncEnabled(settings.vsync);

	view.setSize(((float)width / (float)height) * 150.0F, 150.0F);
	view.setCenter(view.getSize()/2.0F);
	window.setView(view);

	gui.setTarget(window);
}

int Master::loop() {
	while (run) {
		if (currentWindowState == WindowState::MainMenu) {
			MainMenu mainMenu(this);
			fpsCounter = &mainMenu;
			currentCloseable = &mainMenu;
			mainMenu.loop();
		}
		else if (currentWindowState == WindowState::GameClient) {
			ClientGame clientGame(this, hostIp);
			fpsCounter = &clientGame;
			currentCloseable = &clientGame;
			clientGame.loop();
		}
		else if (currentWindowState == WindowState::GameServer) {
			ServerGame serverGame(this);
			fpsCounter = &serverGame;
			currentCloseable = &serverGame;
			serverGame.loop();
		}
	}
	return 0;
}

void Master::launchMainMenu() {
	if (currentCloseable)
		currentCloseable->quit();
	currentWindowState = WindowState::MainMenu;
	createWindow(currentWindowState);
}

void Master::launchClient(std::string hostIp_) {
	if (currentCloseable)
		currentCloseable->quit();
	hostIp = hostIp_;
	currentWindowState = WindowState::GameClient;
	createWindow(currentWindowState);
}

void Master::launchHost() {
	if (currentCloseable)
		currentCloseable->quit();
	currentWindowState = WindowState::GameServer;
	createWindow(currentWindowState);
}



void Master::quit() {
	if (currentCloseable != nullptr) {
		currentCloseable->quit();
	}
	run = false;
}


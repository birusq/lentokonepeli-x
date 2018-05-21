#include "Master.h"
#include "MainMenu.h"
#include "ClientGame.h"
#include "ServerGame.h"
#include "Globals.h"

#define VERSION_NUMBER 1;

Master::Master() {

	console::clearLogFile();
	g::init();

	gui.init(this);

	soundPlayer.init(this);

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
		window.create(sf::VideoMode(width, height), "lentsikat", settings.style, cs);

	if (windowWasOpen == true && settings.fullscreen != true)
		window.setPosition(oldPos);

	if (settings.vsync == false && settings.framerateLimit != 0 && state != WindowState::GameServer) {
		window.setFramerateLimit(settings.framerateLimit);
	}

	if (state != WindowState::GameServer)
		window.setVerticalSyncEnabled(settings.vsync);

	sf::View view;
	view.setSize(((float)width / (float)height) * 150.0F, 150.0F);
	view.setCenter(view.getSize()/2.0F);
	window.setView(view);

	gui.setTarget(window);

	tgui::Clipboard::setWindowHandle(window.getSystemHandle());
}

int Master::loop() {
	while (run) {
		if (currentWindowState == WindowState::MainMenu) {
			MainMenu* mainMenu = new MainMenu();
			fpsCounter = mainMenu;
			currentCloseable = mainMenu;
			mainMenu->loop();
			delete mainMenu;
		}
		else if (currentWindowState == WindowState::GameClient) {
			ClientGame* clientGame = new ClientGame(hostIp);
			fpsCounter = clientGame;
			currentCloseable = clientGame;
   			clientGame->loop();
			delete clientGame;
		}
		else if (currentWindowState == WindowState::GameServer) {
			ServerGame* serverGame = new ServerGame();
			fpsCounter = serverGame;
			currentCloseable = serverGame;
			serverGame->loop();
			delete serverGame;
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
	console::dlog("client");

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


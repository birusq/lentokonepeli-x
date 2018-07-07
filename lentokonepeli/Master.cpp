#include "Master.h"
#include "MainMenu.h"
#include "ClientGame.h"
#include "ServerGame.h"
#include "Globals.h"

void Master::init() {
	console::clearLogFile();
	g::init();

	gui.init();

	soundPlayer.init();
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
		window.create(sf::VideoMode(width, height), "Lentokonepeli-X - Client", settings.style, cs);
	else if (state == WindowState::GameServer)
		window.create(sf::VideoMode(width, height), "Lentokonepeli-X - Server", sf::Style::Titlebar | sf::Style::Close, cs);
	else if (state == WindowState::MainMenu)
		window.create(sf::VideoMode(width, height), "Lentokonepeli-X", settings.style, cs);

	if (windowWasOpen == true && settings.fullscreen != true)
		window.setPosition(oldPos);

	if (settings.vsync == false && settings.framerateLimit != 0 && state != WindowState::GameServer) {
		window.setFramerateLimit(settings.framerateLimit);
	}

	if (state != WindowState::GameServer)
		window.setVerticalSyncEnabled(settings.vsync);

	window.setIcon(128, 128, fileLoader.getImage("lentokonepeli-x_icon.png")->getPixelsPtr());

	sf::View view;
	defaultViewSize = sf::Vector2f(((float)width / (float)height) * 180.0F, 180.0F);
	view.setSize(defaultViewSize);
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
			ClientGame* clientGame = new ClientGame(hostAddress);
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

void Master::launchClient(RakNet::SystemAddress hostAddress_) {
	if (currentCloseable)
		currentCloseable->quit();
	hostAddress = hostAddress_;
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


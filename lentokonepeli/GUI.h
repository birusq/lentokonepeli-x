#pragma once

#include <TGUI\TGUI.hpp>
#include <SFML\Graphics.hpp>

class Server;
class Client;
class Master;

class GUI {
private:

	Master* master;

	tgui::Gui gui;
	
	sf::Clock clock;
	float time = 0;
	void initMainMenu();
	tgui::Panel::Ptr mainMenuPanel;
	tgui::EditBox::Ptr joinIpEditBox;
	tgui::EditBox::Ptr usernameEditBox;

	tgui::Label::Ptr pingLabel;
	tgui::Label::Ptr fpsLabel;

	void hostButtonPressed();
	void clientButtonPressed();
	
	void initServer();
	void showServer();
	tgui::Panel::Ptr sPanel;
	tgui::ChatBox::Ptr sCLI;
	tgui::EditBox::Ptr sCLIInput;
	
	void onCLIInput();
	
	void initClient();
	void showClient();
	tgui::Panel::Ptr cPanel;
	tgui::ChatBox::Ptr chatBox;
	tgui::EditBox::Ptr chatBoxInput;
	
	tgui::Panel::Ptr chooseTeamPanel;

	tgui::Panel::Ptr escMenuPanel;

	tgui::Label::Ptr spawnTimeLabel;

	bool host;

	tgui::Button::Ptr createButton(tgui::Panel::Ptr parent, std::string text = "", unsigned int textSize = 14, tgui::Color bgColor = tgui::Color::White, bool useDarkText = true);

	tgui::Color tint(tgui::Color baseColor, tgui::Color tintColor, float factor);

public:
	void init(Master* master_);
	void setTarget(sf::RenderTarget& target);
	void draw();
	void handleEvent(sf::Event event);

	void teamJoinAccepted();

	void toggleEscMenu();
	void showEscMenu();
	void hideEscMenu();

	// Set float to -1 to show key prompt
	void updateSpawnTimeLabel(bool setVisible, float timer);

	void showMainMenu();

	Server* server;
	Client* client;

	int lastPing = -1;
};
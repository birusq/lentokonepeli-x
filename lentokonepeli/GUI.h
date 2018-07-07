#pragma once

#include <TGUI\TGUI.hpp>
#include <SFML\Graphics.hpp>
#include "GUIPanelList.h"
#include "GUIScoreboard.h"

class Server;
class Client;
class Master;

class GUI {
private:
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
	unsigned int defaultTextSize = 12;
	tgui::EditBox::Ptr chatBoxInput;
	tgui::Label::Ptr spawnTimeLabel;
	tgui::Panel::Ptr scoreboardPanel;

	tgui::Panel::Ptr killFeedPanel;
	std::unique_ptr<GUIPanelList> killFeed;

	tgui::Panel::Ptr pointFeedPanel;
	std::unique_ptr<GUIPanelList> pointFeed;

	bool host;

	tgui::Button::Ptr createButton(tgui::Panel::Ptr parent, std::string text = "", unsigned int textSize = 14, tgui::Color bgColor = tgui::Color::White, bool useDarkText = true);

	tgui::Color tint(tgui::Color baseColor, tgui::Color tintColor, float factor);

	void initGUIScaleSetting(sf::RenderTarget& window);

	void multiplySize(tgui::Container::Ptr container, float multiplier);

	void reloadChatBox(tgui::ChatBox::Ptr chatBox);

	float menuScale = 1.0F;
	float clientScale = 1.0F;

	std::unordered_map<std::string, tgui::Panel::Ptr> panels;

public:
	void init();
	void draw(float dt);
	void setTarget(sf::RenderTarget& window);
	void handleEvent(sf::Event event);

	void teamJoinAccepted();

	void toggleScoreboard();
	void showScoreboard();
	void hideScoreboard();

	void togglePanel(std::string panelName);
	void showPanel(std::string panelName);
	void hidePanel(std::string panelName);

	void showKillFeedMessage(std::string s1, std::string s2, std::string s3, sf::Color s1color, sf::Color s2color, sf::Color s3color);
	void showPointFeedMessage(std::string msg);

	std::unique_ptr<GUIScoreboard> scoreBoard;

	// Set float to -1 to show key prompt
	void updateSpawnTimeLabel(bool setVisible, float timer);

	void showMainMenu();

	void updateScale();

	Server* server;
	Client* client;

	int lastPing = -1;

	bool hidden = false;
};
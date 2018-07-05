#include "GUI.h"
#include <iostream>
#include "Console.h"
#include "Master.h"
#include "Game.h"
#include "Client.h"
#include "Globals.h"
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>

void GUI::init() {

	gui.setFont(g::font);

	if (master->settings.showFps) {
		fpsLabel = tgui::Label::create();
		fpsLabel->getRenderer()->setTextColor(tgui::Color::White);
		//fpsLabel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 100));
		gui.add(fpsLabel);
	}
	if (master->settings.showPing) {
		pingLabel = tgui::Label::create();
		pingLabel->getRenderer()->setTextColor(tgui::Color::White);
		//pingLabel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 100));
		pingLabel->setPosition(0 , 18);
		gui.add(pingLabel);
	}
	clock.restart();
}

void GUI::setTarget(sf::RenderTarget& window) {
	gui.setTarget(window);
	if(master->settings.guiScalePercent < 80) {
		initGUIScaleSetting(window);
	}
}

void GUI::draw(float dt) {
	if(hidden)
		return;

	time += dt;

	if (time >= 0.5F) {
		if (master->settings.showFps) {
			fpsLabel->setText(std::to_string(master->fpsCounter->getCurrentFps()) + " fps");
		}
		if (lastPing != -1 && master->settings.showPing) {
			pingLabel->setText(std::to_string(lastPing) + " ms");
		}
		time -= 0.5F;
	}

	if(killFeed != nullptr) {
		killFeed->updateTimers(dt);
	}
	if(pointFeed != nullptr) {
		pointFeed->updateTimers(dt);
	}

	gui.draw();
}

void GUI::handleEvent(sf::Event event) {
	gui.handleEvent(event);
}

void GUI::updateScale() {
	if(master->settings.guiScalePercent.getValue() < 80) {
		master->settings.guiScalePercent.setValue(80);
	}
	float newScale = (float)master->settings.guiScalePercent.getValue() / 100.0F;
	if(mainMenuPanel) {
		float mp = newScale / menuScale;
		multiplySize(mainMenuPanel, mp);
		menuScale = newScale;
	}
	if(cPanel) {
		float mp = newScale / clientScale;
		multiplySize(cPanel, mp);
		multiplySize(panels["inGameMenu"], mp);
		panels["inGameMenu"]->setSize({ "100%", "100%" });
		multiplySize(panels["chooseTeam"], mp);
		pointFeed->setScale(newScale);
		killFeed->setScale(newScale);
		scoreBoard->setScale(newScale);
		clientScale = newScale;
	}
}

void GUI::multiplySize(tgui::Container::Ptr container, float multiplier) {
	std::vector<tgui::Widget::Ptr> widgets = container->getWidgets();
	for(tgui::Widget::Ptr widget : widgets) {
		auto oldSize = widget->getSize().x;
		widget->setSize(widget->getSize() * multiplier);
		if(auto label = std::dynamic_pointer_cast<tgui::Label>(widget)) {
			label->setTextSize((unsigned int)roundf(label->getTextSize() * multiplier));
			label->setAutoSize(true);
		}
		else if(auto button = std::dynamic_pointer_cast<tgui::Button>(widget)) {
			button->setTextSize((unsigned int)roundf(button->getTextSize() * multiplier));
		}
		else if(auto editBox = std::dynamic_pointer_cast<tgui::EditBox>(widget)) {
			editBox->setTextSize((unsigned int)roundf(editBox->getTextSize() * multiplier));
		}
		else if (auto chatBox = std::dynamic_pointer_cast<tgui::ChatBox>(widget)) {
			chatBox->setTextSize((unsigned int)roundf(chatBox->getTextSize() * multiplier));
			reloadChatBox(chatBox);
		}
	}
}

void GUI::reloadChatBox(tgui::ChatBox::Ptr chatBox) {
	std::vector<std::pair<sf::String, tgui::Color>> lineTexts;
	for(std::size_t i = 0; i < chatBox->getLineAmount(); i++) {
		lineTexts.push_back(std::make_pair(chatBox->getLine(i), chatBox->getLineColor(i)));
	}
	chatBox->removeAllLines();
	for(std::size_t i = 0; i < lineTexts.size(); i++) {
		chatBox->addLine(lineTexts[i].first, lineTexts[i].second);
	}
}

void GUI::initMainMenu() {

	mainMenuPanel = tgui::Panel::create({"100%", "100%"});
	mainMenuPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
	gui.add(mainMenuPanel);
	mainMenuPanel->hide();

	auto logo = tgui::Label::create("Lentokonepeli-X");
	mainMenuPanel->add(logo);
	logo->getRenderer()->setTextColor(tgui::Color::White);
	logo->setTextSize(25);
	logo->setPosition("(&.width - width)/2", "16%");
	
	auto usernameLabel = tgui::Label::create("Choose username");
	usernameLabel->getRenderer()->setTextColor(tgui::Color::White);
	mainMenuPanel->add(usernameLabel);
	usernameLabel->setPosition("(&.width - width)/2", tgui::bindBottom(logo) + 40);

	usernameEditBox = tgui::EditBox::create();
	mainMenuPanel->add(usernameEditBox);
	usernameEditBox->setSize(210, 22);
	usernameEditBox->setMaximumCharacters(20);
	usernameEditBox->setPosition("(&.width - width)/2", tgui::bindBottom(usernameLabel));
	usernameEditBox->setAlignment(tgui::EditBox::Alignment::Center);
	if (master->settings.username.getValue() != "*") {
		usernameEditBox->setText(master->settings.username.getValue());
	}

	auto joinIpLabel = tgui::Label::copy(usernameLabel);
	mainMenuPanel->add(joinIpLabel);
	joinIpLabel->setText("Choose ip to join");
	joinIpLabel->setPosition("(&.width - width)/2", tgui::bindBottom(usernameEditBox) + 40);

	joinIpEditBox = tgui::EditBox::copy(usernameEditBox);
	mainMenuPanel->add(joinIpEditBox);
	joinIpEditBox->setDefaultText("eg. 127.0.0.1#62000");
	joinIpEditBox->setMaximumCharacters(0);
	joinIpEditBox->setText("");
	joinIpEditBox->setPosition("(&.width - width)/2", tgui::bindBottom(joinIpLabel));
	joinIpEditBox->connect("ReturnKeyPressed", [&]() { clientButtonPressed(); });

	auto joinGameButton = createButton(mainMenuPanel, "Join game", 18);
	joinGameButton->setPosition("(&.width - width)/2", tgui::bindBottom(joinIpEditBox) + 5);
	joinGameButton->connect("pressed", [&]() { clientButtonPressed(); });

	auto hostButton = createButton(mainMenuPanel, "Host game", 18);
	hostButton->setPosition("(&.width - width)/2", tgui::bindBottom(joinGameButton) + 40);
	hostButton->connect("pressed", [&]() { hostButtonPressed(); });

	auto guitButton = createButton(mainMenuPanel, "Quit", 18);
	guitButton->setPosition("(&.width - width)/2", tgui::bindBottom(hostButton) + 40);
	guitButton->connect("pressed", [&]() { master->quit(); });

	auto versionLabel = tgui::Label::copy(usernameLabel); 
	mainMenuPanel->add(versionLabel);
	versionLabel->setText("v" + g::version);
	versionLabel->setAutoSize(true);
	versionLabel->setPosition("(&.width - width)/2", "(&.height - height) - 10");

	updateScale();
}

void GUI::showMainMenu() {
	hidden = false;
	if (sPanel) {
		gui.remove(sPanel);
	}
	if (cPanel) {
		gui.remove(cPanel);
	}
	initMainMenu();
	mainMenuPanel->show();
}

void GUI::initGUIScaleSetting(sf::RenderTarget& window) {
	if(window.getSize().x < 720) {
		master->settings.guiScalePercent.setValue(100);
	}
	else {
		master->settings.guiScalePercent.setValue((int)(std::min(window.getSize().x / 1280.0F, window.getSize().y / 720.0F) * 100));
	}
}

void GUI::initServer() {
	sPanel = tgui::Panel::create();
	sPanel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
	gui.add(sPanel);

	sCLIInput = tgui::EditBox::create();
	sPanel->add(sCLIInput);
	sCLIInput->getRenderer()->setBackgroundColor(tgui::Color(49, 52, 58));
	sCLIInput->getRenderer()->setBackgroundColorHover(tgui::Color(49, 52, 58));
	sCLIInput->getRenderer()->setCaretColor(tgui::Color::White);
	sCLIInput->getRenderer()->setTextColor(tgui::Color::White);
	sCLIInput->getRenderer()->setBorderColor(tgui::Color::Black);
	sCLIInput->setSize("&.width - 20", 22);
	sCLIInput->setPosition(10, "&.height - height - 10");
	sCLIInput->setMaximumCharacters(500);
	sCLIInput->connect("ReturnKeyPressed", [&]() { onCLIInput(); });
	sCLIInput->focus();

	auto scrollbar = tgui::ScrollbarRenderer();
	scrollbar.setTrackColor(tgui::Color(255, 255, 255, 20));
	scrollbar.setTrackColorHover(tgui::Color(255, 255, 255, 20));
	scrollbar.setArrowBackgroundColor(tgui::Color(255, 255, 255, 100));

	sCLI = tgui::ChatBox::create();
	sPanel->add(sCLI);
	sCLI->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
	sCLI->getRenderer()->setScrollbar(scrollbar.getData());
	sCLI->setSize("&.width - 20", "&.height - 80");
	sCLI->setPosition(10, 48);
	sCLI->setLineLimit(500);

	console::currentOut = &sCLI;
}

void GUI::showServer() {
	hidden = false;
	if (cPanel) {
		gui.remove(cPanel);
	}
	if (mainMenuPanel) {
		gui.remove(mainMenuPanel);
	}
	initServer();
	sPanel->show();
}

void GUI::initClient() {
	cPanel = tgui::Panel::create();
	cPanel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
	gui.add(cPanel);

	spawnTimeLabel = tgui::Label::create("Press any key to spawn");
	cPanel->add(spawnTimeLabel);
	spawnTimeLabel->getRenderer()->setTextColor(tgui::Color::Red);
	spawnTimeLabel->setTextSize(26);
	spawnTimeLabel->setPosition("&.width/2 - width/2", "80%");
	spawnTimeLabel->hide();

	killFeedPanel = tgui::Panel::create({ 600, 200 });
	cPanel->add(killFeedPanel);
	killFeedPanel->setPosition("&.width - width - 10", 10);
	killFeedPanel->getRenderer()->setBackgroundColor(sf::Color::Transparent);

	killFeed = std::make_unique<GUIPanelList>(killFeedPanel, 5, 3, std::numeric_limits<float>::infinity(), Align::RIGHT);

	pointFeedPanel = tgui::Panel::create({ "100%", 100 });
	cPanel->add(pointFeedPanel);
	pointFeedPanel->setPosition("&.width/2 - width/2", "&.height - 150");
	pointFeedPanel->getRenderer()->setBackgroundColor(sf::Color::Transparent);

	pointFeed = std::make_unique<GUIPanelList>(pointFeedPanel, 5, 3, 3.0F, Align::CENTER);

	auto scrollbar = tgui::ScrollbarRenderer();
	scrollbar.setTrackColor(tgui::Color(0, 0, 0, 20));
	scrollbar.setArrowBackgroundColor(tgui::Color(0, 0, 0, 20));
	scrollbar.setArrowBackgroundColorHover(tgui::Color(255, 255, 255, 20));
	scrollbar.setThumbColor(tgui::Color(0, 0, 0, 50));
	scrollbar.setThumbColorHover(tgui::Color(255, 255, 255, 50));

	chatBox = tgui::ChatBox::create();
	cPanel->add(chatBox);
	chatBox->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 60));
	chatBox->getRenderer()->setBorderColor(tgui::Color(0, 0, 0, 0));
	chatBox->getRenderer()->setScrollbar(scrollbar.getData());
	chatBox->setSize(380, 220);
	chatBox->setPosition(0, "parent.bottom - height");
	chatBox->setLineLimit(100);

	console::currentOut = &chatBox;

	scoreboardPanel = tgui::Panel::create({ 800, 600 });
	cPanel->add(scoreboardPanel);
	scoreboardPanel->setPosition("(&.width - width)/2", "(&.height - height)/2");
	auto greySeethrough = palette::strongGrey;
	greySeethrough.a = 210;
	scoreboardPanel->getRenderer()->setBackgroundColor(greySeethrough);
	scoreboardPanel->hide();

	scoreBoard = std::make_unique<GUIScoreboard>(scoreboardPanel);

	panels["chooseTeam"] = tgui::Panel::create({ 600, 400 });
	cPanel->add(panels["chooseTeam"]);
	panels["chooseTeam"]->setPosition("(&.width - width)/2", "(&.height - height)/2");
	panels["chooseTeam"]->getRenderer()->setBackgroundColor(greySeethrough);
	panels["chooseTeam"]->hide();

	auto chooseTeamLabel = tgui::Label::create("Choose team");
	panels["chooseTeam"]->add(chooseTeamLabel);
	chooseTeamLabel->getRenderer()->setTextColor(tgui::Color::White);
	chooseTeamLabel->setTextSize(20);
	chooseTeamLabel->setPosition("(&.width - width)/2", "20%");

	auto redTeamButton = createButton(panels["chooseTeam"], "Red", 34, palette::red, false);
	redTeamButton->setSize(250, 150);
	redTeamButton->setPosition("&.width/4 - width/2", "40%");
	redTeamButton->connect("pressed", [&]() { client->requestTeamJoin(Team::RED_TEAM); });

	auto blueTeamButton = createButton(panels["chooseTeam"], "Blue", 34, palette::blue, false);
	blueTeamButton->setSize(250, 150);
	blueTeamButton->setPosition("&.width*3/4 - width/2", "40%");
	blueTeamButton->connect("pressed", [&]() { client->requestTeamJoin(Team::BLUE_TEAM); });
	
	panels["inGameMenu"] = tgui::Panel::create();
	cPanel->add(panels["inGameMenu"]);
	panels["inGameMenu"]->hide();
	panels["inGameMenu"]->getRenderer()->setBackgroundColor(greySeethrough);
	panels["inGameMenu"]->setPosition("(&.width - width)/2", "(&.height - height)/2");

	auto resume = createButton(panels["inGameMenu"], "Resume", 20, sf::Color::White, true);
	resume->setPosition("&.width/2 - width/2", "20%");
	resume->connect("pressed", [&]() { hidePanel("inGameMenu"); });

	auto quitToMainMenu = createButton(panels["inGameMenu"], "Exit to main menu", 20, sf::Color::White, true);
	quitToMainMenu->setPosition("&.width/2 - width/2", bindBottom(resume) + bindHeight(resume));
	quitToMainMenu->connect("pressed", [&]() { master->launchMainMenu(); });

	auto quitToDesktop = createButton(panels["inGameMenu"], "Quit to desktop", 20, sf::Color::White, true);
	quitToDesktop->setPosition("&.width/2 - width/2", bindBottom(quitToMainMenu) + bindHeight(quitToMainMenu));
	quitToDesktop->connect("pressed", [&]() { master->quit(); });
	
	updateScale();
}

void GUI::teamJoinAccepted() {
	hidePanel("chooseTeam");
	updateSpawnTimeLabel(true, -1.0F);
}

void GUI::toggleScoreboard() {
	if(scoreboardPanel->isVisible()) {
		hideScoreboard();
	}
	else {
		showScoreboard();
	}
}

void GUI::showScoreboard() {
	if (scoreboardPanel)
		scoreboardPanel->show();
}

void GUI::hideScoreboard() {
	if(scoreboardPanel)
		scoreboardPanel->hide();
}

void GUI::togglePanel(std::string panelName) {
	if(panels.count(panelName) == 1) {
		if(panels[panelName]->isVisible()) {
			panels[panelName]->hide();
		}
		else {
			panels[panelName]->show();
		}
	}
	else {
		console::dlog("Can't toggle gui panel \"" + panelName + "\", it doesn't exist" );
	}
}

void GUI::showPanel(std::string panelName) {
	if(panels.count(panelName) == 1) {
		panels[panelName]->show();
	}
	else {
		console::dlog("Can't show gui panel \"" + panelName + "\", it doesn't exist");
	}
}

void GUI::hidePanel(std::string panelName) {
	if(panels.count(panelName) == 1) {
		panels[panelName]->hide();
	}
	else {
		console::dlog("Can't hide gui panel \"" + panelName + "\", it doesn't exist");
	}
}

void GUI::showKillFeedMessage(std::string s1, std::string s2, std::string s3, sf::Color s1color, sf::Color s2color, sf::Color s3color) {
	int index = killFeed->addItem();
	killFeed->editSubLabel(index, 0, s1, s1color);
	killFeed->editSubLabel(index, 1, s2, s2color);
	killFeed->editSubLabel(index, 2, s3, s3color);
}

void GUI::showPointFeedMessage(std::string msg) {
	int index = pointFeed->addItem();
	pointFeed->editSubLabel(index, 0, msg);
}

void GUI::updateSpawnTimeLabel(bool setVisible, float timer) {
	if (setVisible)
		spawnTimeLabel->show();
	else
		spawnTimeLabel->hide();

	if (timer < -0.5F)
		spawnTimeLabel->setText("Press \"shoot\" key to spawn");
	else {
		std::stringstream ss;
		ss << "Spawning in " << std::fixed << std::setprecision(2) << timer << " s";
		spawnTimeLabel->setText(ss.str());
	}

}

tgui::Button::Ptr GUI::createButton(tgui::Panel::Ptr parent, std::string text, unsigned int textSize, tgui::Color bgColor, bool userDarkText) {
	tgui::Button::Ptr button = tgui::Button::create(text);
	parent->add(button);
	button->setTextSize(textSize);
	tgui::ButtonRenderer* rend = button->getRenderer();

	rend->setBorders(tgui::Borders(1,1,1,1));

	rend->setBackgroundColor(bgColor);

	if (userDarkText) {
		rend->setTextColor(tgui::Color::Black);
		rend->setTextColorHover(tgui::Color::Black);
		rend->setTextColorDown(tgui::Color::Black);
		rend->setTextColorDisabled(tgui::Color(66, 66, 66));
		
		rend->setBorderColor(tgui::Color::Black);
		rend->setBorderColorHover(tgui::Color::Black);
		rend->setBorderColorDown(tgui::Color::Black);
		rend->setBorderColorDisabled(tgui::Color(66, 66, 66));

		rend->setBackgroundColorHover(tint(bgColor, tgui::Color::Black, 0.2F));
		rend->setBackgroundColorDown(tint(bgColor, tgui::Color::Black, 0.3F));
		rend->setBackgroundColorDisabled(tint(bgColor, tgui::Color::Black, 0.5F));
	}
	else {
		rend->setTextColor(tgui::Color::White);
		rend->setTextColorHover(tgui::Color::White);
		rend->setTextColorDown(tgui::Color::White);
		rend->setTextColorDisabled(tgui::Color(219, 219, 219));

		rend->setBorderColor(tgui::Color::White);
		rend->setBorderColorHover(tgui::Color::White);
		rend->setBorderColorDown(tgui::Color::White);
		rend->setBorderColorDisabled(tgui::Color(219, 219, 219));

		rend->setBackgroundColorHover(tint(bgColor, tgui::Color::White, 0.2F));
		rend->setBackgroundColorDown(tint(bgColor, tgui::Color::White, 0.3F));
		rend->setBackgroundColorDisabled(tint(bgColor, tgui::Color::White, 0.5F));
	}
	
	return button;
}

tgui::Color GUI::tint(tgui::Color baseColor, tgui::Color tintColor, float factor)
{
	unsigned int r = (unsigned int)((float)baseColor.getRed() + (float)(tintColor.getRed() - baseColor.getRed()) * factor);
	unsigned int g = (unsigned int)((int)(float)baseColor.getGreen() + (float)(tintColor.getGreen() - baseColor.getGreen()) * factor);
	unsigned int b = (unsigned int)((float)baseColor.getBlue() + (float)(tintColor.getBlue() - baseColor.getBlue()) * factor);
	return tgui::Color(r,g,b);
}

void GUI::showClient() {
	if (sPanel) {
		gui.remove(sPanel);
	}
	if (mainMenuPanel) {
		gui.remove(mainMenuPanel);
	}
	initClient();
	cPanel->show();
}

void GUI::hostButtonPressed() {
	showServer();
	master->launchHost();
	host = true;
}

void GUI::clientButtonPressed() {
	std::string joinAddress = joinIpEditBox->getText();
#ifdef _DEBUG
	if(joinAddress.size() == 0) {
		joinAddress = "127.0.0.1#62000";
	}
#else
	if(joinAddress.size() == 0) {
		joinIpEditBox->getRenderer()->setBorderColor(tgui::Color::Red);
		return;
	}
#endif

	std::size_t pos = joinAddress.find("localhost");
	if(pos != std::string::npos) {
		joinAddress.replace(pos, 9, "127.0.0.1");
	}


	RakNet::SystemAddress address;
	if(address.FromString(joinAddress.c_str(), '#')) {

		sf::String str = usernameEditBox->getText();
		str.replace(" ", "_");

		if(str.getSize() > 0) {
			master->settings.username.setValue(str);
		}
		else {
			master->settings.username.setValue("*");
		}

		showClient();
		master->launchClient(address);
		host = false;
	}
	else {
		joinIpEditBox->getRenderer()->setBorderColor(tgui::Color::Red);
	}
}

void GUI::onCLIInput() {
	sf::String input = sCLIInput->getText();

	sCLIInput->setText("");
}
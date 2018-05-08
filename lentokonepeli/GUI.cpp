#include "GUI.h"
#include <iostream>
#include "Console.h"
#include "Master.h"
#include "Game.h"
#include "Client.h"

void GUI::init(Master* master_) {
	master = master_;

	font = tgui::Font("arial.ttf");

	gui.setFont(font);

	if (master->settings.showFps) {
		fpsLabel = tgui::Label::create();
		fpsLabel->getRenderer()->setTextColor(tgui::Color::White);
		gui.add(fpsLabel);
	}
	if (master->settings.showPing) {
		pingLabel = tgui::Label::create();
		pingLabel->getRenderer()->setTextColor(tgui::Color::White);
		pingLabel->setPosition(0 , 18);
		gui.add(pingLabel);
	}
	
	initMainMenu();
	clock.restart();
}

void GUI::setTarget(sf::RenderTarget& target) {
	gui.setTarget(target);
}

void GUI::draw() {
	float frameTime = clock.restart().asSeconds();
	time += frameTime;

	if (time >= 0.5F) {
		if (master->settings.showFps) {
			fpsLabel->setText(std::to_string(master->fpsCounter->getCurrentFps()) + " fps");
		}
		if (lastPing != -1 && master->settings.showPing) {
			pingLabel->setText(std::to_string(lastPing) + " ms");
		}
		time -= 0.5F;
	}

	gui.draw();
}

void GUI::handleEvent(sf::Event event) {
	gui.handleEvent(event);
}

void GUI::initMainMenu() {
	mainMenuPanel = tgui::Panel::create();
	mainMenuPanel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
	gui.add(mainMenuPanel);

	auto logo = tgui::Label::create("Lentokonepeli X");
	mainMenuPanel->add(logo);
	logo->setTextSize(25);
	logo->setPosition("(&.width - width)/2", "20%");
	
	auto usernameLabel = tgui::Label::create("choose username");
	mainMenuPanel->add(usernameLabel);
	usernameLabel->setPosition("(&.width - width)/2", "35%");

	usernameEditBox = tgui::EditBox::create();
	mainMenuPanel->add(usernameEditBox);
	usernameEditBox->setInputValidator("[a-z���A-Z���0-9_.-]*");
	usernameEditBox->setSize(210, 22);
	usernameEditBox->setMaximumCharacters(20);
	usernameEditBox->setPosition("(&.width - width)/2", tgui::bindBottom(usernameLabel));
	usernameEditBox->setAlignment(tgui::EditBox::Alignment::Center);
	if (master->settings.username.getValue() != "*") {
		usernameEditBox->setText(master->settings.username.getValue());
	}

	auto joinIpLabel = tgui::Label::create("choose ip to join");
	mainMenuPanel->add(joinIpLabel);
	joinIpLabel->setPosition("(&.width - width)/2", "50%");
	joinIpEditBox = tgui::EditBox::copy(usernameEditBox);
	joinIpEditBox->setInputValidator("[a-zA-Z0-9.]*");
	mainMenuPanel->add(joinIpEditBox);
	joinIpEditBox->setMaximumCharacters(0);
	joinIpEditBox->setText("");
	joinIpEditBox->setPosition("(&.width - width)/2", tgui::bindBottom(joinIpLabel));
	joinIpEditBox->connect("ReturnKeyPressed", [&]() { clientButtonPressed(); });

	auto joinGameButton = tgui::Button::create();
	mainMenuPanel->add(joinGameButton);
	joinGameButton->setTextSize(18);
	joinGameButton->setText("Join game");
	joinGameButton->setPosition("(&.width - width)/2", tgui::bindBottom(joinIpEditBox) + "5");
	joinGameButton->connect("pressed", [&]() { clientButtonPressed(); });

	auto hostButton = tgui::Button::copy(joinGameButton);
	mainMenuPanel->add(hostButton);
	hostButton->setText("Host game");
	hostButton->setPosition("(&.width - width)/2", "69%");
	hostButton->connect("pressed", [&]() { hostButtonPressed(); });

	auto guitButton = tgui::Button::copy(joinGameButton);
	mainMenuPanel->add(guitButton);
	guitButton->setText("Quit");
	guitButton->setPosition("(&.width - width)/2", "80%");
	guitButton->connect("pressed", [&]() { master->quit(); });
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

	serverUIInitalized = true;
}

void GUI::showServer() {
	mainMenuPanel->hide();
	if (clientUIInitalized) {
		cPanel->hide();
	}
	if (!serverUIInitalized) {
		initServer();
	}
	sPanel->show();
}

void GUI::initClient() {
	cPanel = tgui::Panel::create();
	cPanel->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
	gui.add(cPanel);

	chatBox = tgui::ChatBox::create();
	cPanel->add(chatBox);
	chatBox->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 100));
	chatBox->getRenderer()->setBorderColor(tgui::Color(0, 0, 0, 0));
	chatBox->setSize(340, 200);
	chatBox->setPosition(0, "parent.bottom - height - 100");
	chatBox->setLineLimit(100);

	console::currentOut = &chatBox;

	chooseTeamPanel = tgui::Panel::create({ "60%", "40%" });
	chooseTeamPanel->setPosition("(&.width - width)/2", "(&.height - height)/2");
	gui.add(chooseTeamPanel);

	auto chooseTeamLabel = tgui::Label::create("Choose team");
	chooseTeamPanel->add(chooseTeamLabel);
	chooseTeamLabel->setTextSize(20);
	chooseTeamLabel->setPosition("(&.width - width)/2", "20%");

	auto redTeamButton = tgui::Button::create("Red");
	chooseTeamPanel->add(redTeamButton);
	redTeamButton->setSize("40%", "40%");
	redTeamButton->setPosition("&.width/4 - width/2", "40%");
	redTeamButton->connect("pressed", [&]() { client->requestTeamJoin(RED_TEAM); });

	auto blueTeamButton = tgui::Button::create("Blue");
	chooseTeamPanel->add(blueTeamButton);
	blueTeamButton->setSize("40%", "40%");
	blueTeamButton->setPosition("&.width*3/4 - width/2", "40%");
	blueTeamButton->connect("pressed", [&]() { client->requestTeamJoin(BLUE_TEAM); });
	
	clientUIInitalized = true;
	// NOT DONE
}

void GUI::teamJoinAccepted() {
	chooseTeamPanel->hide();
}

void GUI::showClient() {
	mainMenuPanel->hide();
	if (serverUIInitalized) {
		sPanel->hide();
	}
	if (!clientUIInitalized) {
		initClient();
	}
	cPanel->show();
}

void GUI::hostButtonPressed() {
	master->launchHost();
	showServer();
	host = true;
}

void GUI::clientButtonPressed() {
	std::string joinIp = joinIpEditBox->getText();
	RakNet::SystemAddress address;
	bool res = address.FromString(joinIp.c_str());
	if (res == true && joinIp.length() != 0) {

		sf::String str = usernameEditBox->getText();

		str.replace(" ", "_");

		if (str.getSize() > 0) {
			master->settings.username.setValue(str);
		}
		else {
			master->settings.username.setValue("*");
		}

		master->launchClient(joinIp);
		showClient();
		host = false;
	}
#ifdef _DEBUG
	// for testing
	else if (joinIp.size() == (size_t)0) {
		sf::String str = usernameEditBox->getText();
		str.replace(" ", "");
		if (str.getSize() > 0) 
			master->settings.username.setValue(str);
		else 
			master->settings.username.setValue("*");
		master->launchClient("192.168.1.40");
		showClient();
		host = false;
	}
#endif
	else {
		std::cout << joinIp << " is not a valid ip address\n";
	}
}

void GUI::onCLIInput() {
	sf::String input = sCLIInput->getText();

	sCLIInput->setText("");
}
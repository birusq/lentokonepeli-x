#include "GUI.h"
#include <iostream>
#include "Console.h"
#include "Master.h"
#include "Game.h"
#include "Client.h"
#include "Globals.h"

void GUI::init(Master* master_) {
	master = master_;

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
	mainMenuPanel = tgui::Panel::create({"100%", "100%"});
	mainMenuPanel->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
	gui.add(mainMenuPanel);

	auto logo = tgui::Label::create("Lentokonepeli X");
	mainMenuPanel->add(logo);
	logo->getRenderer()->setTextColor(tgui::Color::White);
	logo->setTextSize(25);
	logo->setPosition("(&.width - width)/2", "20%");
	
	auto usernameLabel = tgui::Label::create("choose username");
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

	auto joinIpLabel = tgui::Label::create("choose ip to join");
	mainMenuPanel->add(joinIpLabel);
	joinIpLabel->setPosition("(&.width - width)/2", tgui::bindBottom(usernameEditBox) + 40);

	joinIpEditBox = tgui::EditBox::copy(usernameEditBox);
	mainMenuPanel->add(joinIpEditBox);
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
	chatBox->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 80));
	chatBox->getRenderer()->setBorderColor(tgui::Color(0, 0, 0, 0));
	chatBox->setSize(340, 200);
	chatBox->setPosition(0, "parent.bottom - height - 100");
	chatBox->setLineLimit(100);

	console::currentOut = &chatBox;

	chooseTeamPanel = tgui::Panel::create({ "60%", "40%" });
	chooseTeamPanel->setPosition("(&.width - width)/2", "(&.height - height)/2");
	chooseTeamPanel->getRenderer()->setBackgroundColor(tgui::Color(palette::strongGrey));
	gui.add(chooseTeamPanel);

	auto chooseTeamLabel = tgui::Label::create("Choose team");
	chooseTeamPanel->add(chooseTeamLabel);
	chooseTeamLabel->getRenderer()->setTextColor(tgui::Color::White);
	chooseTeamLabel->setTextSize(20);
	chooseTeamLabel->setPosition("(&.width - width)/2", "20%");

	auto redTeamButton = createButton(chooseTeamPanel, "Red", 24, palette::red, false);
	redTeamButton->setSize("40%", "40%");
	redTeamButton->setPosition("&.width/4 - width/2", "40%");
	redTeamButton->connect("pressed", [&]() { client->requestTeamJoin(RED_TEAM); });

	auto blueTeamButton = createButton(chooseTeamPanel, "Blue", 24, palette::blue, false);
	blueTeamButton->setSize("40%", "40%");
	blueTeamButton->setPosition("&.width*3/4 - width/2", "40%");
	blueTeamButton->connect("pressed", [&]() { client->requestTeamJoin(BLUE_TEAM); });
	
	clientUIInitalized = true;
	// NOT DONE
}

void GUI::teamJoinAccepted() {
	chooseTeamPanel->hide();
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
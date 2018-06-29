#pragma once
#include "MainMenu.h"
#include "FpsCounter.h"
#include "Master.h"
#include "Globals.h"

MainMenu::MainMenu() {
	gui = &master->gui;
	gui->showMainMenu();
}

void MainMenu::loop() {
	auto& window = master->window;

	window.setMouseCursorVisible(true);

	sf::Clock clock;

	while (window.isOpen() && running) {

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				quit();
				master->quit();
			}
			gui->handleEvent(event);
		}

		float frameTime = clock.restart().asSeconds();
		updateFrameTimes(frameTime);

		render(window, frameTime);
	}
}

void MainMenu::render(sf::RenderWindow& w, float dt) {
	w.clear(sf::Color(palette::strongGrey));

	gui->draw(dt);

	w.display();
}


void MainMenu::quit() {
	running = false;
}

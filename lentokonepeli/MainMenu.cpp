#pragma once
#include "MainMenu.h"
#include "FpsCounter.h"
#include "Master.h"

MainMenu::MainMenu(Master* master_) : FpsCounter(master_) {
	master = master_;
	gui = &master->gui;
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

		render(window);
	}
}

void MainMenu::render(sf::RenderWindow& w) {
	w.clear(sf::Color(74, 186, 80));

	gui->draw();

	w.display();
}


void MainMenu::quit() {
	running = false;
}

#pragma once

#include "TGUI/TGUI.hpp"

class GUI;

enum Align {
	LEFT,
	RIGHT,
	CENTER,
};

class GUIPanelList {
public:
	GUIPanelList() {}
	GUIPanelList(tgui::Panel::Ptr parentPanel_, unsigned int maxItemCount_, unsigned int subLabelCount_, float itemTimeOut_, Align alignment_);

	void updateTimers(float dt);

	// add item and return it's index
	int addItem();
	void editSubLabel(unsigned int itemIndex, unsigned int subLabelIndex, std::string text, sf::Color color = sf::Color::White);

private:
	GUI* myGui;

	tgui::Panel::Ptr parentPanel;
	std::vector<tgui::Panel::Ptr> itemPanels;
	std::vector<float> itemPanelTimers;

	tgui::Panel::Ptr itemTemplate;

	unsigned int maxItemCount;
	unsigned int subLabelCount;
	float itemTimeOut;
	Align alignment;
};

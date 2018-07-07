#pragma once

#include "TGUI/TGUI.hpp"
#include <limits>

class GUI;

enum Align {
	LEFT,
	RIGHT,
	CENTER
};

class GUIPanelList {
public:
	GUIPanelList() {}
	GUIPanelList(tgui::Panel::Ptr parentPanel_, unsigned int maxItemCount_, unsigned int subLabelCount_, float itemTimeOut_ = std::numeric_limits<float>::infinity(), Align alignment_ = Align::LEFT, unsigned int itemDefaultTextSize_ = 14);

	void updateTimers(float dt);

	// add item and return it's index
	int addItem(tgui::Panel::Ptr customTemplate = nullptr);
	void editSubLabel(unsigned int itemIndex, unsigned int subLabelIndex, std::string text, sf::Color color = sf::Color::White);

	int currentItemCount() { return itemPanels.size(); }

	void removeItem(int index);

	void setScale(float newScale);

	// Only edit itemTemplate after initalization
	tgui::Panel::Ptr itemTemplate;

	bool labelsFormContinousLine = true;

private:
	tgui::Panel::Ptr parentPanel;
	std::vector<tgui::Panel::Ptr> itemPanels;
	std::vector<float> itemPanelTimers;

	unsigned int maxItemCount;
	unsigned int subLabelCount;
	float itemTimeOut;
	Align alignment;

	sf::Vector2f defaultParentSize;
	unsigned int itemDefaultTextSize = 14;

	float scale = 1.0F;
	
	void applyCurrentScale(tgui::Panel::Ptr panel);

	void repositionPanels();
};

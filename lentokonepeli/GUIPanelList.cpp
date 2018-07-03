#include "GUIPanelList.h"
#include "Console.h"
#include <Thor/Math.hpp>

GUIPanelList::GUIPanelList(tgui::Panel::Ptr parentPanel_, unsigned int maxItemCount_, unsigned int subLabelCount_, float itemTimeOut_, Align alignment_, unsigned int itemDefaultTextSize_)
	: parentPanel{ parentPanel_ }, maxItemCount{ maxItemCount_ }, subLabelCount{ subLabelCount_ }, itemTimeOut{ itemTimeOut_ }, alignment{ alignment_ }, itemDefaultTextSize{ itemDefaultTextSize_ } {

	defaultParentSize = sf::Vector2f(parentPanel->getSize());

	itemTemplate = tgui::Panel::create();
	itemTemplate->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 80));

	for(unsigned int i = 0; i < subLabelCount; i++) {
		auto label = tgui::Label::create();
		label->setTextSize(itemDefaultTextSize);
		itemTemplate->add(label, std::to_string(i));
	}
}

void GUIPanelList::updateTimers(float dt) {
	for(std::size_t i = 0; i < itemPanels.size(); i++) {
		itemPanelTimers[i] -= dt;
		if(itemPanelTimers[i] <= 0.0F) {
			parentPanel->remove(itemPanels[i]);
			itemPanels.erase(itemPanels.begin() + i);
			itemPanelTimers.erase(itemPanelTimers.begin() + i);
		}
	}
}

int GUIPanelList::addItem(tgui::Panel::Ptr customTemplate) {
	if(itemPanels.size() == maxItemCount) {
		removeItem(0);
	}
	tgui::Panel::Ptr newPanel = nullptr;
	if (customTemplate != nullptr)
		newPanel = tgui::Panel::copy(customTemplate);
	else 
		newPanel = tgui::Panel::copy(itemTemplate);

	parentPanel->add(newPanel);
	itemPanels.push_back(newPanel);
	itemPanelTimers.push_back(itemTimeOut);

	tgui::Label::Ptr lastLabel = nullptr;
	tgui::Layout panelWidth;

	if(labelsFormContinousLine) {
		for(unsigned int i = 0; i < subLabelCount; i++) {
			tgui::Label::Ptr label = newPanel->get<tgui::Label>(std::to_string(i));
			label->setTextSize((unsigned int)((float)itemDefaultTextSize * scale));
			if(i == 0) {
				label->setPosition(3, label->getTextSize() * .4F);
			}
			else {
				label->setPosition(tgui::bindRight(lastLabel) - 3, label->getTextSize() * .4F);
			}
			lastLabel = label;
			auto tempPanelWidth = panelWidth + tgui::bindWidth(label) - 3;
			panelWidth = tempPanelWidth;
		}
		auto tempPanelWidth = panelWidth + 9;
		panelWidth = tempPanelWidth;
	}
	else {
		for(unsigned int i = 0; i < subLabelCount; i++) {
			tgui::Label::Ptr label = newPanel->get<tgui::Label>(std::to_string(i));
			label->setTextSize((unsigned int)((float)itemDefaultTextSize * scale));
			lastLabel = label;
		}
		panelWidth = tgui::bindWidth(parentPanel);
	}

	float height = (float)itemDefaultTextSize * scale * 2.0F;

	newPanel->setSize(panelWidth, height);

	repositionPanels();

	return itemPanels.size() - 1;
}

void GUIPanelList::editSubLabel(unsigned int itemIndex, unsigned int subLabelIndex, std::string text, sf::Color color) {
	if((int)itemIndex > (int)itemPanels.size() - 1) {
		throw std::invalid_argument("itemIndex too high, it doesn't exist");
	}
	auto labelPtr = itemPanels[itemIndex]->get<tgui::Label>(std::to_string(subLabelIndex));
	labelPtr->setText(text);
	labelPtr->getRenderer()->setTextColor(color);
}

void GUIPanelList::removeItem(int index) {
	parentPanel->remove(itemPanels[index]);
	itemPanels.erase(itemPanels.begin() + index);
	itemPanelTimers.erase(itemPanelTimers.begin() + index);

	repositionPanels();
}

void GUIPanelList::setScale(float newScale) {
	scale = newScale;
	parentPanel->setSize(defaultParentSize * scale);
	reloadAll();
}

void GUIPanelList::reloadAll() {
	parentPanel->removeAllWidgets();
	std::vector<tgui::Panel::Ptr> tempItemPanels = itemPanels;
	itemPanels.clear();

	for(std::size_t i = 0; i < tempItemPanels.size(); i++) {
		addItem(tempItemPanels[i]);
	}
}

void GUIPanelList::repositionPanels() {
	float height = (float)itemDefaultTextSize * scale * 2.0F;
	for(std::size_t i = 0; i < itemPanels.size(); i++) {
		if(alignment == Align::RIGHT) {
			itemPanels[i]->setPosition("&.width - width", i * height);
		}
		else if(alignment == Align::LEFT) {
			itemPanels[i]->setPosition(0, i * height);
		}
		else if(alignment == Align::CENTER) {
			itemPanels[i]->setPosition("&.width/2 - width/2", i * height);
		}
	}
}

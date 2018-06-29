#include "GUIPanelList.h"
#include "Console.h"

GUIPanelList::GUIPanelList(tgui::Panel::Ptr parentPanel_, unsigned int maxItemCount_, unsigned int subLabelCount_, float itemTimeOut_, Align alignment_)
	: parentPanel{parentPanel_}, maxItemCount { maxItemCount_}, subLabelCount{ subLabelCount_ }, itemTimeOut{ itemTimeOut_ }, alignment{ alignment_ } {

	itemTemplate = tgui::Panel::create();
	itemTemplate->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 80));

	for(unsigned int i = 0; i < subLabelCount; i++) {
		auto label = tgui::Label::create();
		label->setTextSize(14);
		itemTemplate->add(label, std::to_string(i));
	}
}

void GUIPanelList::updateTimers(float dt) {
	for(std::size_t i = 0; i < itemPanelTimers.size(); i++) {
		itemPanelTimers[i] -= dt;
		if(itemPanelTimers[i] <= 0.0F) {
			parentPanel->remove(itemPanels[i]);
			itemPanels.erase(itemPanels.begin() + i);
			itemPanelTimers.erase(itemPanelTimers.begin() + i);
		}
	}
}

int GUIPanelList::addItem() {
	if(itemPanels.size() == maxItemCount) {
		parentPanel->remove(itemPanels[0]);
		itemPanels.erase(itemPanels.begin());
		itemPanelTimers.erase(itemPanelTimers.begin());
	}
	auto newPanel = tgui::Panel::copy(itemTemplate);
	parentPanel->add(newPanel);
	itemPanels.push_back(newPanel);
	itemPanelTimers.push_back(itemTimeOut);

	tgui::Label::Ptr lastLabel = nullptr;
	tgui::Layout panelWidth;
	for(unsigned int i = 0; i < subLabelCount; i++) {
		tgui::Label::Ptr label = newPanel->get<tgui::Label>(std::to_string(i));
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

	float height = lastLabel->getTextSize() * 2.0F;

	newPanel->setSize(panelWidth, height);

	for(std::size_t i = 0; i < itemPanels.size(); i++) {
		itemPanels[i]->setPosition(0, i * lastLabel->getTextSize() + 6);
		if(alignment == Align::RIGHT) {
			itemPanels[i]->setPosition("&.width - width", i * height);
		}
		else if(alignment == Align::LEFT) {
			itemPanels[i]->setPosition(0, i * lastLabel->getTextSize() + 6);
		}
		else if(alignment == Align::CENTER) {
			itemPanels[i]->setPosition("&.width/2 - width/2", i * height);
		}
	}

	//newPanel->setPosition(bindRight(parentPanel) - bindWidth(newPanel), itemPanels.size() * 20);

	return itemPanels.size() - 1;
}

void GUIPanelList::editSubLabel(unsigned int itemIndex, unsigned int subLabelIndex, std::string text, sf::Color color) {
	if((int)itemIndex > (int)itemPanels.size() - 1) {
		throw std::invalid_argument("itemIndex too high, it doesn't exist");
	}
	auto labelPtr = itemPanels[itemIndex]->get<tgui::Label>(std::to_string(subLabelIndex));
	labelPtr->setText(text);
	labelPtr->getRenderer()->setTextColor(color);

	console::stream << subLabelIndex << labelPtr->getPosition().x << ", " << labelPtr->getPosition().y;
	console::dlogStream();
}

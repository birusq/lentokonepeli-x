#include "GUIScoreboard.h"
#include "Globals.h"
#include <limits>
#include "Console.h"

GUIScoreboard::GUIScoreboard(tgui::Panel::Ptr parentPanel_): parentPanel{ parentPanel_ } {
	
	for(int i = 1; i < 3; i++) {
		Team::Id teamId = (Team::Id)i;
		listPanelParents[teamId] = tgui::ScrollablePanel::create({ "49.9%", "90%" });

		auto scrollbar = tgui::ScrollbarRenderer();
		scrollbar.setTrackColor(tgui::Color(255, 255, 255, 20));
		scrollbar.setTrackColorHover(tgui::Color(255, 255, 255, 20));
		scrollbar.setArrowBackgroundColor(tgui::Color(255, 255, 255, 100));

		listPanelParents[teamId]->getRenderer()->setScrollbar(scrollbar.getData());
		listPanelParents[teamId]->setHorizontalScrollbarPolicy(tgui::ScrollablePanel::ScrollbarPolicy::Never);
		listPanelParents[teamId]->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		parentPanel->add(listPanelParents[teamId]);
		if(i == 1) {
			auto label = tgui::Label::create("RED TEAM");
			parentPanel->add(label);
			label->getRenderer()->setTextColor(palette::red);
			label->setPosition("&.width/4 - width/2", "&.height * 0.05 - height/2");
			label->setTextSize(20);
			listPanelParents[teamId]->setPosition(0, "9%");
		}
		else if(i == 2) {
			auto label = tgui::Label::create("BLUE TEAM");
			parentPanel->add(label);
			label->getRenderer()->setTextColor(palette::blue);
			label->setPosition("&.width/4 * 3 - width/2", "&.height * 0.05 - height/2");
			label->setTextSize(20);
			listPanelParents[teamId]->setPosition("50.1%", "9%");
		}
		listPanels[teamId] = GUIPanelList(listPanelParents[teamId], 255, 5, std::numeric_limits<float>::infinity(), Align::LEFT, 12);
		listPanels[teamId].labelsFormContinousLine = false;
		tgui::Panel::Ptr itemTemp = listPanels[teamId].itemTemplate;

		auto label = itemTemp->get<tgui::Label>("0");
		label->setPosition(label->getTextSize() * 0.2F, label->getTextSize() * 0.4F);
		label->setSize("&.width * 0.45", label->getSize().y);
		itemTemp->get<tgui::Label>("1")->setPosition("&.width * 0.50 - width/2", label->getTextSize() * 0.4F);
		itemTemp->get<tgui::Label>("2")->setPosition("&.width * 0.60 - width/2", label->getTextSize() * 0.4F);
		itemTemp->get<tgui::Label>("3")->setPosition("&.width * 0.70 - width/2", label->getTextSize() * 0.4F);
		itemTemp->get<tgui::Label>("4")->setPosition("&.width * 0.85 - width/2", label->getTextSize() * 0.4F);
		int index = listPanels[teamId].addItem();
		listPanels[teamId].editSubLabel(index, 0, "", sf::Color(140, 140, 140));
		listPanels[teamId].editSubLabel(index, 1, "K", sf::Color(140, 140, 140));
		listPanels[teamId].editSubLabel(index, 2, "D", sf::Color(140, 140, 140));
		listPanels[teamId].editSubLabel(index, 3, "A", sf::Color(140, 140, 140));
		listPanels[teamId].editSubLabel(index, 4, "P", sf::Color(140, 140, 140));
	}
}

void GUIScoreboard::updateScores(std::unordered_map<sf::Uint8, UserScore>& userScores, std::unordered_map<Team::Id, std::vector<sf::Uint8>>& sortedUsers, NetworkAgent& networkAgent) {
	for(auto& pair : sortedUsers) {
		Team::Id teamId = pair.first;
		std::vector<sf::Uint8>& clientIds = pair.second;
		for(int i = 0; i < (int)clientIds.size(); i++) {
			int index = i + 1; // do this because first item is just "K-D-A"-indicators
			if(index >= listPanels[teamId].currentItemCount()) {
				index = listPanels[teamId].addItem();
			}
			listPanels[teamId].editSubLabel(index, 0, networkAgent.users[clientIds[i]].username.C_String());
			listPanels[teamId].editSubLabel(index, 1, std::to_string(userScores[clientIds[i]].kills));
			listPanels[teamId].editSubLabel(index, 2, std::to_string(userScores[clientIds[i]].deaths));
			listPanels[teamId].editSubLabel(index, 3, std::to_string(userScores[clientIds[i]].assists));
			listPanels[teamId].editSubLabel(index, 4, std::to_string(userScores[clientIds[i]].points));
		}

		while((int)clientIds.size() + 1 > listPanels[teamId].currentItemCount()) {
			listPanels[teamId].removeItem(listPanels[teamId].currentItemCount() - 1);
		}
	}
}

void GUIScoreboard::setScale(float newScale) {
	float multiplier = newScale / scale;
	std::vector<tgui::Widget::Ptr> widgets = parentPanel->getWidgets();
	for(tgui::Widget::Ptr widget : widgets) {
		if(auto label = std::dynamic_pointer_cast<tgui::Label>(widget)) {
			label->setTextSize((unsigned int)roundf(label->getTextSize() * multiplier));
			label->setAutoSize(true);
		}
	}
	for(auto pair : listPanels) {
		pair.second.setScale(newScale);
	}
	scale = newScale;
}

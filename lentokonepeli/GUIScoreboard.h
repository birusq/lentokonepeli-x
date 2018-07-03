#pragma once

#include "Scores.h"
#include "Team.h"
#include <TGUI/TGUI.hpp>
#include "GUIPanelList.h"
#include <unordered_map>
#include <vector>
#include "NetworkAgent.h"

class GUIScoreboard {
public:
	GUIScoreboard() {}
	GUIScoreboard(tgui::Panel::Ptr parentPanel_);
	
	~GUIScoreboard() {}

	void updateScores(std::unordered_map<sf::Uint8, UserScore>& userScores, std::unordered_map<Team::Id, std::vector<sf::Uint8>>& sortedUsers, NetworkAgent& networkAgent);

	void setScale(float newScale);

private:
	std::unordered_map<Team::Id, tgui::ScrollablePanel::Ptr> listPanelParents;
	std::unordered_map<Team::Id, GUIPanelList> listPanels;
	tgui::Panel::Ptr parentPanel;

	float scale = 1.0;
};
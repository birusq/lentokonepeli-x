#pragma once
#include "TGUI\TGUI.hpp"
#include "SFML\System.hpp"
#include <unordered_map>
#include <iostream>

namespace console {

	extern tgui::ChatBox::Ptr* currentOut;

	void log(std::string s);

	void dlog(std::string s);

	void clearLogFile();
}
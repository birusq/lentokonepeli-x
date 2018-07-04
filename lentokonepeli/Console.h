#pragma once
#include "TGUI\TGUI.hpp"
#include "SFML\System.hpp"
#include <unordered_map>
#include <iostream>

namespace console {

	extern tgui::ChatBox::Ptr* currentOut;

	void log(std::string s);

	void dlog(std::string s);

	// log the current stream
	void log();

	// debug log the current stream
	void dlogStream();

	// log the current stream
	void logStream();

	extern std::ostringstream stream;

	void clearLogFile();

	extern sf::Uint16 logIndex;
}
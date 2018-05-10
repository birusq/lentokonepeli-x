#include "Console.h"
#include <fstream>

namespace console {
	void log(std::string s) {
		if (currentOut != nullptr && s != "")
			(*currentOut)->addLine(s, tgui::Color(224, 202, 35));

		std::fstream file;
		file.open("log.txt", std::ios::app);
		file << s << "\n";
		file.close();
	}

	void dlog(std::string s) {
		if (s != "")
			log("[debug] " + s);
	}

	void clearLogFile() {
		std::ofstream file("log.txt");
		file.close();
	}

	tgui::ChatBox::Ptr* currentOut;
}
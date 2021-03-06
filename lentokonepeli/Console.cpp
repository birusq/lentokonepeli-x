#include "Console.h"

namespace console {
	void log(std::string s) {
		if(currentOut != nullptr && s != "") {
			(*currentOut)->addLine(s, tgui::Color::White);
		}

		std::fstream file;
		file.open("log.txt", std::ios::app);
		file << s << "\n";
		file.close();
	}

	void dlog(std::string s) {
		if (s != "") {
			logIndex++;
			log("[d" + std::to_string(logIndex) + "] " + s);
		}
	}

	void clearLogFile() {
		std::ofstream file("log.txt");
		file.close();
	}


	void log() {
		log(stream.str());
		stream.str("");
	}

	void dlogStream() {
		dlog(stream.str());
		stream.str("");
	}

	void logStream() {
		log(stream.str());
		stream.str("");
	}

	std::ostringstream stream;

	tgui::ChatBox::Ptr* currentOut;

	sf::Uint16 logIndex = -1;
}
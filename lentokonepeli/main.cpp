#include "Master.h"

Master* master;

int main() {
	master = new Master();
	master->init();
	master->launchMainMenu();
	master->loop();
	delete master;
	return 0;
}
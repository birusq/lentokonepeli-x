#include "Master.h"

int main() {
	std::unique_ptr<Master> master = std::make_unique<Master>();
	return master->loop();
}
#include "Master.h"

Master* master;

int main() {
	master = new Master();
	master->loop();
}
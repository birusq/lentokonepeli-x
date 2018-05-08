#pragma once

class Closeable {
public:
	bool running = true;
	virtual void quit() = 0;
};
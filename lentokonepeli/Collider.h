#pragma once

class Collider {
	virtual bool collision(Collider& other) = 0;
};
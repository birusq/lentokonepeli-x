#pragma once

class Damageable {
public:
	Damageable(float startingHealth) : maxHealth{ startingHealth }, health{ startingHealth } {}
	Damageable() {}

	float maxHealth = 100;
	float health = 0;

	void setHealthToFull() { health = maxHealth; }

	const bool isDead() const { return (health <= 0); }

	virtual void takeDmg(float dmg) = 0;

	virtual void restoreHealth(float heal) = 0;

	virtual void onDeath() = 0;
};
#pragma once

class Damageable {
public:
	enum DamageType : sf::Uint8 {
		DMG_UNIDENTIFIED,
		DMG_BULLET,
		DMG_SHIP_COLLISION,
		DMG_SNEAKY //show no visuals and play no audio
	};

	Damageable(int startingHealth) : maxHealth{ startingHealth }, health{ startingHealth } {}
	Damageable() {}

	int maxHealth = 100;
	int health = 0;

	void setHealthToFull() { health = maxHealth; }

	const bool isDead() const { return (health <= 0); }

	virtual void takeDmg(int dmg, DamageType dmgType = DMG_UNIDENTIFIED, sf::Uint8 damageDealer = 0) = 0;

	virtual void restoreHealth(int heal) = 0;

	virtual void onDeath() = 0;
};
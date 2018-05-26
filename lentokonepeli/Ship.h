#pragma once
#include <SFML\Graphics.hpp>
#include "PhysicsTransformable.h"
#include "Damageable.h"
#include "Collider.h"
#include "Team.h"
#include "Weapon.h"

struct User;

class Ship : public PhysicsTransformable, public Collider<sf::RectangleShape>, public Damageable {
public:
	Ship() {}
	Ship(sf::Uint32 pTransId_, User* user_, Team::Id teamId_ = Team::NO_TEAM);

	void assignTeam(Team::Id teamId_);

	bool hasTeam() const { return (teamId != Team::NO_TEAM); }

	User* owner = nullptr;

	Team::Id teamId;

	void takeDmg(int dmg, DamageType dmgType = DMG_UNIDENTIFIED) override;

	void restoreHealth(int heal) override;

	void onDeath() override;

	void draw(sf::RenderTarget& target);

	// heals to full and enables hitbox
	void respawn();

	void onCollision() override;
	void updateHitbox() override;
	void setWeaponTrans(sf::Vector2f pos, float rot);

	std::unique_ptr<Weapon> weapon;

	sf::Clock timeSinceDeath;

	bool localPlayer = false;

	bool throttle = false;

	float turnSpeed = 180.0F;
	float turnSmoothingFrames = 4.0F;

	int bodyHitDamage = 5;

private:

	sf::Text usernameLabel;

	sf::RectangleShape shipBody;

	sf::RectangleShape exhaust;

	sf::RectangleShape healthBar;
	sf::RectangleShape healthBarBG;
	float hbMaxLength = 14.0F;
	float hbBorderSize = 0.5F;

	sf::Clock dmgTimer;
	float dmgTime = 0.04F;
};
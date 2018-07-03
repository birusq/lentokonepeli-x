#pragma once
#include <SFML\Graphics.hpp>
#include "PhysicsTransformable.h"
#include "Damageable.h"
#include "Collider.h"
#include "Team.h"
#include "Weapon.h"
#include "FlickerCountdownTimer.h"
#include <map>
#include "Scores.h"

struct User;
class Game;

class Ship : public PhysicsTransformable, public Collider<sf::RectangleShape>, public Damageable {
public:
	Ship() {}
	Ship(Game* game_, sf::Uint32 pTransId_, User* user_, Team::Id teamId_ = Team::NO_TEAM);

	Game* game = nullptr;

	void assignTeam(Team::Id teamId_);

	bool hasTeam() const { return (teamId != Team::NO_TEAM); }

	User* owner = nullptr;

	Team::Id teamId;

	void takeDmg(int dmg, DamageType dmgType = DMG_UNIDENTIFIED, sf::Uint8 dmgDealer = 0) override;

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
	sf::Clock timeAlive;

	bool throttle = false;

	bool localPlayer = false;

	// This ship instance is running in a server (not in client machine)
	bool inServer = false;

	float turnSpeed = 180.0F;
	float turnSmoothingFrames = 4.0F;

	int bodyHitDamage = 40;

	FlickerCountdownTimer bodyHitImmunityTimer = FlickerCountdownTimer(0.4F, 0.1F);

	float getFuel() { return fuel; }

	// Who have dealt damage to this ship in the last x seconds
	std::vector<DmgContributor> dmgContributors;
	float assistTimeLimit = 6.0F;

	void setBodyTexture(float rotation);

private:

	float fuel = 100.0F;

	sf::Text usernameLabel;

	std::shared_ptr<sf::Texture> bodyTexture;
	sf::Sprite body;

	std::shared_ptr<sf::Texture> exhaustFlameTexture;
	sf::Sprite exhaustFlame;
	sf::Clock flickerTimer;
	float flickerInterval = 0.05F;

	sf::RectangleShape healthBar;
	sf::RectangleShape healthBarBG;
	float hbMaxLength = 14.0F;
	float hbBorderSize = 0.5F;

	CountdownTimer visualDmgTimer = CountdownTimer(0.04F);

	FlickerCountdownTimer respawnAnimTimer = FlickerCountdownTimer(0.6F, 0.1F);
};
#pragma once
#include <SFML\Graphics.hpp>
#include "PhysicsTransformable.h"
#include "Damageable.h"
#include "PacketHelper.h"
#include "Team.h"
#include "Weapon.h"
#include "Collider.h"

struct User;

class Ship : public PhysicsTransformable, public Collider, public Damageable {
public:
	Ship() {}
	Ship(sf::Uint32 pTransId_, User* const user_, TeamId teamId_ = NO_TEAM);

	void assignTeam(TeamId teamId_);

	bool hasTeam() const { return (teamId != NO_TEAM); }

	User* owner = nullptr;

	TeamId teamId;

	void takeDmg(float dmg) override;

	void restoreHealth(float heal) override;

	void onDeath() override;

	void draw(sf::RenderTarget& target);

	// Updates the positions of the weapon and hitbox
	void updateHitbox();

	std::unique_ptr<Weapon> weapon;

private:

	sf::Text usernameLabel;

	sf::RectangleShape rectangle;
};
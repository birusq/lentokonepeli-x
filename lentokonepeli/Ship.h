#pragma once
#include <SFML\Graphics.hpp>
#include "PhysicsTransformable.h"
#include "RectangleCollider.h"
#include "Damageable.h"
#include "PacketHelper.h"
#include "User.h"

class Ship : public PhysicsTransformable, public RectangleCollider, public Damageable {
public:
	Ship() {}
	Ship(User* const user_, TeamId teamId_ = NO_TEAM);

	void assignTeam(TeamId teamId_);

	bool hasTeam() const { return (teamId != NO_TEAM); }

	User* owner = nullptr;

	TeamId teamId;

	void takeDmg(float dmg) override;

	void restoreHealth(float heal) override;

	void onDeath() override;

	void draw(sf::RenderTarget& target);

private:

	sf::Text usernameLabel;

	sf::RectangleShape rectangle;
};
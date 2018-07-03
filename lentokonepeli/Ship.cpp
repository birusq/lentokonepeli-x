#include "Ship.h"
#include "Globals.h"
#include "Console.h"
#include "User.h"
#include "Weapon.h"
#include "Master.h"
#include "PacketHelper.h"
#include "DefaultGun.h"
#include "ServerGame.h"

Ship::Ship(Game* game_, sf::Uint32 pTransId_, User* owner_, Team::Id teamId_) : owner{ owner_ }, game{ game_ }, teamId{ teamId_ } {
	weapon = std::make_unique<DefaultGun>(&(game->goManager), owner->clientId);

	if(dynamic_cast<ServerGame*>(game) != nullptr) {
		inServer = true;
	}

	pTransId = pTransId_;
	gravity = true;
	drag = 0.01F;

	float width = 1.7F;
	float height = 10.0f;

	hitbox.setSize(sf::Vector2f(width, height));
	hitbox.setOrigin(width / 2.0f, height / 2.0F);

	if(!inServer) {
		usernameLabel.setFont(g::font);
		usernameLabel.setString(owner->username.C_String());
		usernameLabel.setCharacterSize(40);
		usernameLabel.setScale(sf::Vector2f(0.065F, 0.065F));
		usernameLabel.setOrigin(usernameLabel.getLocalBounds().width / 2.0F, usernameLabel.getLocalBounds().height);
		usernameLabel.setFillColor(sf::Color::Black);

		healthBar = sf::RectangleShape(sf::Vector2f(hbMaxLength, 0.8F));

		healthBarBG = sf::RectangleShape(sf::Vector2f(healthBar.getSize().x + hbBorderSize * 2.0F, healthBar.getSize().y + hbBorderSize * 2.0F));
		healthBarBG.setFillColor(palette::strongGrey);
	}
	assignTeam(teamId);
}

void Ship::assignTeam(Team::Id teamId_) {
	teamId = teamId_;

	if(!inServer) {
		if(teamId == Team::RED_TEAM) {
			healthBar.setFillColor(palette::red);
		}
		else if(teamId == Team::BLUE_TEAM) {
			healthBar.setFillColor(palette::blue);
		}
		std::shared_ptr<sf::Texture> tex = master->fileLoader.getTexture("ship_atlas.png");
		if(tex != nullptr) {
			tex->setSmooth(true);
			bodyTexture = tex;
			body.setTexture(*bodyTexture);
			body.setOrigin(sf::Vector2f(60, 50));
			body.setScale(0.1F, 0.1F);
		}
		tex = master->fileLoader.getTexture("exhaustFlame_atlas.png");
		if(tex != nullptr) {
			tex->setSmooth(true);
			exhaustFlameTexture = tex;
			exhaustFlame.setTexture(*exhaustFlameTexture);
			exhaustFlame.setOrigin(82, 16);
			exhaustFlame.setScale(0.1F, 0.1F);
		}
	}
}

void Ship::draw(sf::RenderTarget& target) {
	if (localPlayer) {
		master->soundPlayer.updateListenerPos(getPosition());
	}
	
	if (isDead() == false) {
		bool flicker = false;
		if (static_cast<int>(ceilf(flickerTimer.getElapsedTime().asSeconds() / flickerInterval)) % 2 == 1) {
			flicker = true;
		}

		weapon->setPosition(getRotationVector() * 5.0F + getPosition());
		weapon->setRotation(getRotation());

		//weapon->draw(target);

		body.setPosition(getPosition());
		body.setRotation(getRotation());

		if (respawnAnimTimer.flickerOff() && bodyHitImmunityTimer.flickerOff() && visualDmgTimer.isDone()) {
			setBodyTexture(getRotation());
			target.draw(body);
		}

		if (throttle) {
			master->soundPlayer.playThrottle(getPosition(), owner->clientId);
			exhaustFlame.setPosition(getPosition());
			exhaustFlame.setRotation(getRotation() - 90.0F);
			if(flicker)
				exhaustFlame.setTextureRect(sf::IntRect(0,0,32,32));
			else
				exhaustFlame.setTextureRect(sf::IntRect(32, 0, 32, 32));
			target.draw(exhaustFlame);
		}
		else {
			master->soundPlayer.stopThrottle(owner->clientId);
		}

		healthBar.setPosition(getPosition().x - hbMaxLength / 2.0F, getPosition().y - 10.0F);
		healthBarBG.setPosition(healthBar.getPosition().x - hbBorderSize, healthBar.getPosition().y - hbBorderSize);

		usernameLabel.setPosition(getPosition().x, getPosition().y - 12.0F);

		if(master->gui.hidden == false) {
			target.draw(healthBarBG);
			target.draw(healthBar);
			target.draw(usernameLabel);
		}
	}
	else {
		timeAlive.restart();
		master->soundPlayer.stopThrottle(owner->clientId);
	}
}

void Ship::updateHitbox() {
	if(isDead()) {
		weapon->shipFullyAlive = false;
		return;
	}

	hitbox.setRotation(getRotation());
	hitbox.setPosition(getPosition());


	// Weapon can shoot when fully alive
	// Call this here so it gets called both from client and server
	if(timeAlive.getElapsedTime().asSeconds() > 0.2F) {
		weapon->shipFullyAlive = true;
	}
}

void Ship::setWeaponTrans(sf::Vector2f pos, float rot) {
	setRotation(rot);
	weapon->setRotation(rot);
	weapon->setPosition(getRotationVector() * 5.0F + pos);
}

void Ship::setBodyTexture(float rotation) {
	if(rotation >= 337.5F || rotation < 22.5F) {
		body.setTextureRect(sf::IntRect(0, 0, 120, 100));
		body.setScale(0.1F, 0.1F);
	}
	else if(rotation >= 22.5F && rotation < 67.5F) {
		body.setTextureRect(sf::IntRect(122, 0, 120, 100));
		body.setScale(0.1F, 0.1F);
	}
	else if(rotation >= 67.5F && rotation < 112.5F) {
		body.setTextureRect(sf::IntRect(244, 0, 120, 100));
		body.setScale(0.1F, 0.1F);
	}
	else if(rotation >= 112.5F && rotation < 157.5F) {
		body.setTextureRect(sf::IntRect(366, 0, 120, 100));
		body.setScale(0.1F, 0.1F);
	}
	else if(rotation >= 157.5F && rotation < 202.5F) {
		body.setTextureRect(sf::IntRect(488, 0, 120, 100));
		body.setScale(0.1F, 0.1F);
	}
	else if(rotation >= 202.5F && rotation < 247.5F) {
		body.setTextureRect(sf::IntRect(366, 0, 120, 100));
		body.setScale(-0.1F, 0.1F);
	}
	else if(rotation >= 247.5F && rotation < 292.5F) {
		body.setTextureRect(sf::IntRect(244, 0, 120, 100));
		body.setScale(-0.1F, 0.1F);
	}
	else if(rotation >= 292.5F && rotation < 337.5F) {
		body.setTextureRect(sf::IntRect(122, 0, 120, 100));
		body.setScale(-0.1F, 0.1F);
	}
}

void Ship::onCollision() {
	
}

void Ship::respawn() {
	setHealthToFull();
	hitboxDisabled = false;
	healthBar.setSize(sf::Vector2f(health / maxHealth * hbMaxLength, healthBar.getSize().y));
	respawnAnimTimer.start();
	timeAlive.restart();
	dmgContributors.clear();
}

void Ship::takeDmg(int dmg, DamageType dmgType, sf::Uint8 dmgDealer) {
	if(timeAlive.getElapsedTime().asSeconds() < 0.2F) {
		return;
	}

	//maybe react differently to different damage types
	if (dmgType == Damageable::DMG_SHIP_COLLISION) {
		bodyHitImmunityTimer.start();
	}

	console::stream << owner->username.C_String() << " took " << dmg << " damage";
	console::dlogStream();
	health -= dmg;

	if(inServer) {
		if(dmgType == Damageable::DMG_SHIP_COLLISION || dmgType == Damageable::DMG_BULLET) {
			DmgContributor contributor = DmgContributor(dmgDealer, dmg, CountdownTimer());
			for(auto it = dmgContributors.begin(); it != dmgContributors.end();) {
				if(it->clientId == dmgDealer) {
					if(it->timer.isDone() == false) {
						contributor.dmg += it->dmg;
					}
					it = dmgContributors.erase(it);
				}
				else {
					it++;
				}
			}
			dmgContributors.insert(dmgContributors.begin(), contributor);
			dmgContributors[0].timer.start(assistTimeLimit + 100);
		}
	}

	if (health <= 0.0F)
		onDeath();

	if (dmgType != Damageable::DMG_SNEAKY) {
		visualDmgTimer.start();
		master->soundPlayer.playSound(getPosition(), "hurt");
	}
	healthBar.setSize(sf::Vector2f((float)health / (float)maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::restoreHealth(int heal) {
	console::stream << owner->username.C_String() << " healed " << heal << " health";
	console::dlogStream();
	health += heal;
	if (health > maxHealth) {
		health = maxHealth;
	}

	healthBar.setSize(sf::Vector2f((float)health / (float)maxHealth * hbMaxLength, healthBar.getSize().y));
}

void Ship::onDeath() {
	hitboxDisabled = true;
	timeSinceDeath.restart();
	if(inServer) {
		for(auto it = dmgContributors.begin(); it != dmgContributors.end();) {
			if(it->timer.isDone()) {
				it = dmgContributors.erase(it);
			}
			else {
				//console::stream << (int)it->clientId << " contributed in killing " << (int)owner->clientId << " with " << it->dmg << " dmg";
				//console::dlogStream();
				it++;
			}
		}
	}

	game->onShipDeath(this);
}
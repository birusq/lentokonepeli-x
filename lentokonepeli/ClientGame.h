#pragma once

#include "Game.h"
#include "Client.h"
#include "TestLevel.h"


class ClientGame : public Game {
public:
	ClientGame(RakNet::SystemAddress hostAddress_);

	void loop();

	Client client;

	// Fired when user data (clientId etc.) is received from server
	void onConnectionComplete();

	void onOtherUserConnect(User& user);

	void beforeOtherUserDisconnect(User& user);

	void onTeamJoin(sf::Uint8 clientId, Team::Id newTeam);

	void onDamage(DamageMessage& dmg);

	void onSpawnScheduled(sf::Uint8 clientId, float timeLeft);

	// Init ships for players that connected before you
	void onShipInit(ShipInitMessage& shipInitMsg);

	void onShipDeath(Ship* ship) override;
	void onReceiveKillDetails(KillDetails& killDetails);

protected:
	void onBulletCollision(Bullet& bullet, Ship& targetShip) override;
	void onShipToShipCollision(Ship& s1, Ship& s2) override;
	void onShipToGroundCollision(Ship& ship) override;
	void onQuit() override;
private:

	void applyServerStates(ServerShipStates& sss);

	void render(sf::RenderWindow& window, float dt);

	void drawMinimap(sf::RenderWindow& window);

	Input processInput();

	InputResponse applyInput(Input input, Ship& ship, float dt);

	void fixedUpdate(float dt);

	void update(float frameTime, float alpha);

	// Doesn't create new one, just makes current one come alive
	void spawnShip(sf::Uint8 clientId) override;

	sf::Clock spawnRequestTimer = sf::Clock();
	float spawnRequestBlockDuration = 1.0F;

	void handleSpawnTimers(float dt) override;

	void handleKeyEvents(sf::Event& event);

	RakNet::SystemAddress hostAddress;

	// Let gui scale affect also gui drawn with sfml
	void upateSFMLGUIScale();

	sf::Vector2f minimapSizeScreenFactor;

	float guiScale = 1.0F;

	bool takeScreenShot = false;
};
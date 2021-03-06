#pragma once
#include "Ship.h"
#include "PhysicsTransformable.h"

#define SERVER_DEFAULT_PORT 62000

using namespace RakNet;

namespace ph {
	unsigned char getPacketIdentifier(Packet *p);

	enum MessageIndetifier : sf::Uint8 {
		ID_USER_UPDATE = ID_USER_PACKET_ENUM,
		ID_USER_DISCONNECT,
		ID_JOIN_TEAM_REQUEST,
		ID_JOIN_TEAM_FAILED_TEAM_FULL,
		ID_TEAM_UPDATE,
		ID_SHIP_UPDATE,
		ID_SPAWN_REQUEST,
		ID_SPAWN_AFTER_TIME,
		ID_SPAWN_NOT_ALLOWED,
		ID_DAMAGE_DEALT,
		ID_SHIP_INIT,
		ID_KILL_DETAILS,
		ID_SCORES_UPDATE
	};

	std::string msgIDToString(MessageIndetifier id);

	static bool seqGreaterThan(sf::Uint16 seq1, sf::Uint16 seq2) {
		return ((seq1 > seq2) && (seq1 - seq2 <= 32768)) ||
			((seq1 < seq2) && (seq2 - seq1  > 32768));
	}
}

struct ShipState {

	void serialize(BitStream& bitStream, bool write);

	void applyToPTrans(PhysicsTransformable& pTrans) const;

	void generateFromPTrans(const PhysicsTransformable& ship);

	sf::Vector2f position;
	float rotation = 0.0F;
	sf::Vector2f velocity;
	float angularVelocity = 0.0F;

	bool throttle = false;
	bool dead = true;
	bool shoot = false;
	bool moveAbility = false;
	sf::Uint16 bulletId;
};

struct ServerShipStates {
	void serialize(BitStream& bitStream, bool write);
	std::unordered_map<sf::Uint8, ShipState> states;
};

struct DamageMessage {
	DamageMessage() {}
	DamageMessage(sf::Uint8 dealerId_, sf::Uint16 damage_, sf::Uint8 targetId_, Damageable::DamageType damageType_) 
		: dealerId{ dealerId_ }, targetId{ targetId_ }, damage{ damage_ }, damageType{ damageType_ } {}

	void serialize(BitStream& bitStream, bool write);

	sf::Uint8 dealerId;
	sf::Uint8 targetId;
	sf::Uint16 damage;
	Damageable::DamageType damageType;
};

/* 
Sent to new player after they have connected
Contains starting data of a single already connected ship
*/
struct ShipInitMessage {

	void serialize(BitStream& bitStream, bool write);

	bool dead = false;
	sf::Uint8 clientId;
	int health = 0;
	// maybe more stuff later like fuel
};


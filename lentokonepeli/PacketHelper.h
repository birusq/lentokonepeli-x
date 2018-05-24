#pragma once

#include "Raknet\MessageIdentifiers.h"
#include "Raknet\RakPeerInterface.h"
#include "SFML\System.hpp"
#include <unordered_map>
#include <bitset>
#include "Raknet\RakWString.h"
#include "Raknet\BitStream.h"
#include <unordered_map>
#include <unordered_set>
#include "Ship.h"
#include "PhysicsTransformable.h"

#define SERVER_PORT 65000

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
		ID_CAN_SPAWN,
		ID_DAMAGE_DEALT_BULLET,
		ID_DAMAGE_DEALT_SHIP_COLLISION
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
	sf::Uint16 bulletId;
};

struct ServerShipStates {
	void serialize(BitStream& bitStream, bool write);
	std::unordered_map<sf::Uint8, ShipState> states;
};

struct BulletDamage {

	void serialize(BitStream& bitStream, bool write);

	sf::Uint8 shooterId;
	sf::Uint8 targetId;
	sf::Uint16 bulletId;
	sf::Uint16 damage;
	sf::Uint16 newHealth;
	float bulletLifetime;
};

struct ShipsCollisionDamage {

	void serialize(BitStream& bitStream, bool write);

	sf::Uint8 clientId1;
	sf::Uint16 dmgTo1;
	sf::Uint16 newHealth1;
	sf::Uint8 clientId2;
	sf::Uint16 dmgTo2;
	sf::Uint16 newHealth2;
};


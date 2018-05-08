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

typedef unsigned char uchar;
typedef unsigned int uint;

using namespace RakNet;

namespace ph {
	unsigned char getPacketIdentifier(Packet *p);

	enum MessageIndetifier : uchar {
		ID_USER_UPDATE = ID_USER_PACKET_ENUM,
		ID_USER_DISCONNECT,
		ID_JOIN_TEAM_REQUEST,
		ID_JOIN_TEAM_FAILED_TEAM_FULL,
		ID_JOIN_TEAM_ACCEPTED,
		ID_TEAM_UPDATE,
		ID_SHIP_UPDATE,
		ID_CAN_SPAWN,
	};

	std::string msgIDToString(MessageIndetifier id);
}

struct ShipState {

	static void serialize(BitStream& bitStream, ShipState& shipState, bool write);

	static void applyToPTrans(ShipState& shipState, PhysicsTransformable& pTrans);

	static ShipState generateFromPTrans(PhysicsTransformable& ship);

	sf::Vector2f position;
	float rotation = 0.0F;
	sf::Vector2f velocity;
	float angularVelocity = 0.0F;

	bool shooting = false;
	bool dead = true;
};

struct ServerShipStates {
	static void serialize(BitStream& bitStream, ServerShipStates& shipStates, bool write);
	std::unordered_map<uchar, ShipState> states;
};
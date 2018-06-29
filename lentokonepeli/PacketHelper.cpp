#include "PacketHelper.h"
#include "Console.h"

namespace ph {
	unsigned char getPacketIdentifier(RakNet::Packet *p)
	{
		if ((unsigned char)p->data[0] == ID_TIMESTAMP)
			return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned long)];
		else
			return (unsigned char)p->data[0];
	}

	std::string msgIDToString(MessageIndetifier id) {
		switch (id) {
		case ID_USER_UPDATE:
			return "ID_SHIP_UPDATE";
		case ID_USER_DISCONNECT:
			return "ID_USER_DISCONNECT";
		case ID_JOIN_TEAM_REQUEST:
			return "ID_JOIN_TEAM_REQUEST";
		case ID_JOIN_TEAM_FAILED_TEAM_FULL:
			return "ID_JOIN_TEAM_FAILED_TEAM_FULL";
		case ID_TEAM_UPDATE:
			return "ID_TEAM_UPDATE";
		case ID_SHIP_UPDATE:
			return "ID_SHIP_UPDATE";
		default:
			return "id " + std::to_string((unsigned char)id);
		}
	}
}

void ShipState::serialize(RakNet::BitStream& bitStream, bool write) {
	bitStream.Serialize(write, dead);
	if (!dead) {
		bitStream.Serialize(write, position.x);
		bitStream.Serialize(write, position.y);
		bitStream.Serialize(write, velocity.x);
		bitStream.Serialize(write, velocity.y);
		bitStream.Serialize(write, rotation);
		bitStream.Serialize(write, angularVelocity);
		bitStream.Serialize(write, throttle);
		bitStream.Serialize(write, shoot);
		if (shoot) {
			bitStream.Serialize(write, bulletId);
		}
	}
}

void ShipState::applyToPTrans(PhysicsTransformable & ship) const {
	ship.setPosition(position);
	ship.setRotation(rotation);
	ship.velocity = velocity;
	ship.angularVelocity = angularVelocity;
}

void ShipState::generateFromPTrans(const PhysicsTransformable& ship) {
	position = ship.getPosition();
	rotation = ship.getRotation();
	velocity = ship.velocity;
	angularVelocity = ship.angularVelocity;
}

void ServerShipStates::serialize(RakNet::BitStream& bitStream, bool write) {
	sf::Uint8 size = 0;
	if (write) {
		size = (sf::Uint8)states.size();
	}

	bitStream.Serialize(write, size);

	if (write) {
		for (auto& pair : states) {
			bitStream.Write(pair.first);
			pair.second.serialize(bitStream, true);
		}
	}
	else {

		states.clear();

		for (sf::Uint8 i = 0; i < size; i++) {
			sf::Uint8 clientId;
			bitStream.Read(clientId);
			ShipState ss;
			ss.serialize(bitStream, false);

			states[clientId] = ss;
		}
	}
}

void DamageMessage::serialize(BitStream & bitStream, bool write) {
	bitStream.Serialize(write, dealerId);
	bitStream.Serialize(write, targetId);
	bitStream.Serialize(write, damage);
	bitStream.Serialize(write, damageType);
}

void ShipInitMessage::serialize(BitStream & bitStream, bool write) {
	bitStream.Serialize(write, clientId);
	bitStream.Serialize(write, health);
}

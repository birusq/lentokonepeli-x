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
		case ID_CAN_SPAWN:
			return "ID_CAN_SPAWN";
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

void BulletDamage::serialize(BitStream & bitStream, bool write) {
	bitStream.Serialize(write, shooterId);
	bitStream.Serialize(write, targetId);
	bitStream.Serialize(write, bulletId);
	bitStream.Serialize(write, damage);
	bitStream.Serialize(write, newHealth);

	if (write) {
		sf::Uint16 temp_bulletLifetime = (sf::Uint16)(bulletLifetime * 100.0F);
		bitStream.Write(temp_bulletLifetime);
	}
	else {
		sf::Uint16 temp_bulletLifetime;
		bitStream.Read(temp_bulletLifetime);
		bulletLifetime = (float)temp_bulletLifetime / 100.0F;
	}
}

void ShipsCollisionDamage::serialize(BitStream & bitStream, bool write) {
	bitStream.Serialize(write, clientId1);
	bitStream.Serialize(write, dmgTo1);
	bitStream.Serialize(write, newHealth1);
	bitStream.Serialize(write, clientId2);
	bitStream.Serialize(write, dmgTo2);
	bitStream.Serialize(write, newHealth2);
}

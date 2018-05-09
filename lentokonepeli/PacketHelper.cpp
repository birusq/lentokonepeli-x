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
	}
}

void ShipState::applyToPTrans(PhysicsTransformable & ship) {
	ship.setPosition(position);
	ship.setRotation(rotation);
	ship.velocity = velocity;
	ship.angularVelocity = angularVelocity;
}

ShipState ShipState::generateFromPTrans(PhysicsTransformable& ship) {
	ShipState ss;
	ss.position = ship.getPosition();
	ss.rotation = ship.getRotation();
	ss.velocity = ship.velocity;
	ss.angularVelocity = ship.angularVelocity;
	return ss;
}

void ServerShipStates::serialize(RakNet::BitStream& bitStream, ServerShipStates& shipStates, bool write) {
	sf::Uint8 size = 0;
	if (write) {
		size = (sf::Uint8)shipStates.states.size();
	}

	bitStream.Serialize(write, size);

	if (write) {
		for (auto& pair : shipStates.states) {
			bitStream.Write(pair.first);
			pair.second.serialize(bitStream, true);
		}
	}
	else {

		shipStates.states.clear();

		for (sf::Uint8 i = 0; i < size; i++) {
			sf::Uint8 clientId;
			bitStream.Read(clientId);
			ShipState ss;
			ss.serialize(bitStream, false);

			shipStates.states[clientId] = ss;
		}
	}
}
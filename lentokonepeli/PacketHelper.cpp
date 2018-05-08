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
		case ID_JOIN_TEAM_ACCEPTED:
			return "ID_JOIN_TEAM_ACCEPTED";
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

void ShipState::serialize(RakNet::BitStream& bitStream, ShipState& shipState, bool write) {
	bitStream.Serialize(write, shipState.dead);
	if (!shipState.dead) {
		bitStream.Serialize(write, shipState.position.x);
		bitStream.Serialize(write, shipState.position.y);
		bitStream.Serialize(write, shipState.velocity.x);
		bitStream.Serialize(write, shipState.velocity.y);
		bitStream.Serialize(write, shipState.rotation);
		bitStream.Serialize(write, shipState.angularVelocity);
		bitStream.Serialize(write, shipState.shooting);
	}
}

void ShipState::applyToPTrans(ShipState & shipState, PhysicsTransformable & ship) {
	ship.setPosition(shipState.position);
	ship.setRotation(shipState.rotation);
	ship.velocity = shipState.velocity;
	ship.angularVelocity = shipState.angularVelocity;
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
	uchar size = 0;
	if (write) {
		size = (uchar)shipStates.states.size();
	}

	bitStream.Serialize(write, size);

	if (write) {
		for (auto& pair : shipStates.states) {
			bitStream.Write(pair.first);
			ShipState::serialize(bitStream, pair.second, true);
		}
	}
	else {

		shipStates.states.clear();

		for (uchar i = 0; i < size; i++) {
			uchar clientId;
			bitStream.Read(clientId);
			ShipState ss;
			ShipState::serialize(bitStream, ss, false);

			shipStates.states[clientId] = ss;
		}
	}
}
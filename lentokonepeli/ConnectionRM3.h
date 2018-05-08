#pragma once

#include "Raknet\ReplicaManager3.h"

using namespace RakNet;

class ConnectionRM3 : public Connection_RM3
{
public:
	ConnectionRM3(const SystemAddress &_systemAddress, RakNetGUID _guid) : Connection_RM3(_systemAddress, _guid) {}
	virtual Replica3 *AllocReplica(RakNet::BitStream *allocationIdBitstream, ReplicaManager3 *replicaManager3);
};
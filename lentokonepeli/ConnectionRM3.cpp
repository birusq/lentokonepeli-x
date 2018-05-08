#include "ConnectionRM3.h"
#include "Server.h"
#include "User.h"

Replica3* ConnectionRM3::AllocReplica(RakNet::BitStream *allocationIdBitstream, ReplicaManager3 *replicaManager3) {
	char objectType; 
	allocationIdBitstream->Read(objectType); 
	if (objectType == ID_USER) 
		return new User; 
	return 0; 
}
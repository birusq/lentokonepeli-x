#include "User.h"

void  User::serialize(RakNet::BitStream& bitStream, User& user, bool write) {
	bitStream.Serialize(write, user.clientId);
	bitStream.Serialize(write, user.guid);
	bitStream.Serialize(write, user.username);
}
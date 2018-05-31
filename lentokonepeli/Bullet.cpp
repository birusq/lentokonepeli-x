#include "Bullet.h"

Bullet::Bullet(GOManager* goManager_, sf::Uint32 pTransId_, sf::Uint8 clientId_, sf::Uint16 bulletId_) {
	goManager = goManager_;
	pTransId = pTransId_;
	clientId = clientId_;
	bulletId = bulletId_;
}
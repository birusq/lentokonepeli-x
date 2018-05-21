#include "SoundPlayer.h"
#include "Master.h"
#include "Console.h"
#include <Thor/Vectors.hpp>

void SoundPlayer::init(Master* master) {
	sf::Listener::setGlobalVolume((float)master->settings.masterVolume);
	buffers["throttle"].loadFromFile("res/sound/throttle.wav");
	buffers["shoot"].loadFromFile("res/sound/shoot.wav");
}

void SoundPlayer::playThrottle(const sf::Vector2f& pos, int id) {
	if (throttleSounds.count(id) == 0) {
		throttleSounds[id].setBuffer(buffers["throttle"]);
		throttleSounds[id].setLoop(true);
		throttleSounds[id].setVolume(50);
		throttleSounds[id].setAttenuation(0.7F);
		throttleSounds[id].setMinDistance(10.0F);
	}
	sf::Sound& sound = throttleSounds.at(id);

	if (abs(pos.x - sf::Listener::getPosition().x) < 0.5F && abs(pos.y - sf::Listener::getPosition().z) < 0.5F) {
		if (!sound.isRelativeToListener()) {
			sound.setRelativeToListener(true);
		}
		sound.setPosition(0, 0, 0);
	}
	else {
		if (sound.isRelativeToListener()) {
			sound.setRelativeToListener(false);
		}
		sound.setPosition(pos.x, 0, pos.y);
	}

	if (sound.getStatus() != sf::SoundSource::Status::Playing) {
		sound.play();
	}

	counter2++;
	if (counter2 == 10) {
		counter2 = 0;
	}
}
	

void SoundPlayer::stopThrottle(int id) {
	if (throttleSounds.count(id) == 0)
		return;

	throttleSounds.at(id).pause();
}

void SoundPlayer::playSound(const sf::Vector2f& pos, std::string name) {
	deleteOldSounds();
	sf::Sound sound(buffers.at(name));
	sounds.push_back(sound);
}

void SoundPlayer::deleteOldSounds() {
	for (auto it = sounds.begin(); it != sounds.end();) {
		if (it->getStatus() != sf::SoundSource::Status::Playing) {
			it = sounds.erase(it);
		}
		else {
			++it;
		}
	}
}

void SoundPlayer::updateListenerPos(const sf::Vector2f & pos) {
	sf::Listener::setPosition(pos.x, 0, pos.y);
	
}

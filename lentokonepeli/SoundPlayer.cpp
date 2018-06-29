#include "SoundPlayer.h"
#include "Master.h"
#include "Console.h"
#include <Thor/Vectors.hpp>

void SoundPlayer::init() {
	sf::Listener::setGlobalVolume((float)master->settings.masterVolumePercent);
	buffers["throttle"].loadFromFile(path + "throttle.wav");
	buffers["shoot"].loadFromFile(path + "shoot.wav");
	buffers["hurt"].loadFromFile(path + "hurt.wav");
}

void SoundPlayer::playThrottle(const sf::Vector2f& pos, int id) {
	if (throttleSounds.count(id) == 0) {
		throttleSounds[id].setBuffer(buffers["throttle"]);
		throttleSounds[id].setLoop(true);
		throttleSounds[id].setVolume(10);
		throttleSounds[id].setAttenuation(0.7F);
		throttleSounds[id].setMinDistance(10.0F);
	}
	sf::Sound& sound = throttleSounds.at(id);

	sound.setPosition(pos.x, 0, pos.y);
	applySpacializationCutoff(sound);

	if (sound.getStatus() != sf::SoundSource::Status::Playing) {
		sound.play();
	}
}
	

void SoundPlayer::stopThrottle(int id) {
	if (throttleSounds.count(id) == 0)
		return;

	throttleSounds.at(id).pause();
}

void SoundPlayer::playSound(const sf::Vector2f& pos, std::string name) {
	deleteOldSounds();
	sounds.push_back(sf::Sound(buffers.at(name)));
	sf::Sound& sound = sounds.front();
	sound.setPosition(pos.x, 0, pos.y);
	applySpacializationCutoff(sound);
	sound.setAttenuation(0.7F);
	sound.setMinDistance(20.0F);
	sound.setVolume(100);
	sound.play();
}

void SoundPlayer::deleteOldSounds() {
	for (auto it = sounds.begin(); it != sounds.end();) {
		if (it->getStatus() != sf::SoundSource::Status::Playing) {
			it = sounds.erase(it);
		}
		else {
			it++;
		}
	}
}

void SoundPlayer::applySpacializationCutoff(sf::Sound & sound) {
	if (abs(sound.getPosition().x - sf::Listener::getPosition().x) < 6.0F && abs(sound.getPosition().z - sf::Listener::getPosition().z) < 6.0F) {
		if (!sound.isRelativeToListener()) {
			sound.setRelativeToListener(true);
		}
		sound.setPosition(0, 0, 0);
	}
	else {
		if (sound.isRelativeToListener()) {
			sound.setRelativeToListener(false);
		}
	}
}

void SoundPlayer::updateListenerPos(const sf::Vector2f & pos) {
	sf::Listener::setPosition(pos.x, 0, pos.y);
}

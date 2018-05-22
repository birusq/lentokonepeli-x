#pragma once
#include <Thor/Vectors.hpp>
#include <vector>
#include <SFML/Graphics.hpp>

template <class S> class Collider {
public:
	Collider() {}
	~Collider() {};
	S hitbox;
	bool hitboxDisabled = true;

	template <class T>
	bool collidesWith(Collider<T>& other) {
		if (hitboxDisabled || other.hitboxDisabled)
			return false;

		std::vector<sf::Vector2f> myNormals = getNormals();
		std::vector<sf::Vector2f> otherNormals = other.getNormals();

		for (std::size_t i = 0; i < myNormals.size(); i++) {
			std::pair<float, float> res1 = getMinMax(myNormals.at(i));
			std::pair<float, float> res2 = other.getMinMax(myNormals.at(i));

			if (std::get<0>(res1) > std::get<1>(res2) || std::get<0>(res2) > std::get<1>(res1))
				return false;
		}

		for (std::size_t i = 0; i < otherNormals.size(); i++) {
			std::pair<float, float> res1 = getMinMax(otherNormals.at(i));
			std::pair<float, float> res2 = other.getMinMax(otherNormals.at(i));

			if (std::get<0>(res1) > std::get<1>(res2) || std::get<0>(res2) > std::get<1>(res1))
				return false;
		}

		onCollision();
		other.onCollision();
		return true;
	}

	// Internal, don't call
	virtual void onCollision() = 0;

	// Only server checks collisions (clients don't call this)
	virtual void updateHitbox() = 0;

	std::vector<sf::Vector2f> getNormals() {
		std::vector<sf::Vector2f> normals;
		normals.reserve(hitbox.getPointCount());

		for (std::size_t i = 0; i < hitbox.getPointCount(); i++) {
			sf::Vector2f p1 = hitbox.getTransform().transformPoint(hitbox.getPoint(i));
			sf::Vector2f p2 = hitbox.getTransform().transformPoint(hitbox.getPoint(i + 1 == hitbox.getPointCount() ? 0 : i + 1));

			sf::Vector2f normal = sf::Vector2f(-p1.y + p2.y, p1.x - p2.x);

			normals.push_back(normal);
		}

		return normals;
	}

	std::pair<float, float> getMinMax(const sf::Vector2f& normal) {

		float minProj = thor::dotProduct(hitbox.getTransform().transformPoint(hitbox.getPoint(0)), normal);
		float maxProj = minProj;

		for (std::size_t i = 1; i < hitbox.getPointCount(); i++) {
			float currProj = thor::dotProduct(hitbox.getTransform().transformPoint(hitbox.getPoint(i)), normal);

			if (currProj < minProj) {
				minProj = currProj;
			}
			if (currProj > maxProj) {
				maxProj = currProj;
			}
		}

		return std::make_pair(minProj, maxProj);
	}
};
#pragma once
#include "Thor/Vectors.hpp"
#include <vector>
#include <SFML/Graphics.hpp>

class Collider {
public:
	Collider() {}
	~Collider() {};
	sf::RectangleShape hitbox;

	bool collidesWith(Collider& target) {
		std::vector<sf::Vector2f> myNormals = getNormals();
		std::vector<sf::Vector2f> otherNormals = target.getNormals();

		for (std::size_t i = 0; i < myNormals.size(); i++) {
			std::pair<float, float> res1 = getMinMax(myNormals.at(i));
			std::pair<float, float> res2 = target.getMinMax(myNormals.at(i));

			if (std::get<0>(res1) > std::get<1>(res2) || std::get<0>(res2) > std::get<1>(res1))
				return false;
		}

		for (std::size_t i = 0; i < otherNormals.size(); i++) {
			std::pair<float, float> res1 = getMinMax(otherNormals.at(i));
			std::pair<float, float> res2 = target.getMinMax(otherNormals.at(i));

			if (std::get<0>(res1) > std::get<1>(res2) || std::get<0>(res2) > std::get<1>(res1))
				return false;
		}

		return true;
	}
protected:
	virtual void onCollision() {}
private:

	std::vector<sf::Vector2f> getNormals() {
		std::vector<sf::Vector2f> normals;
		normals.reserve(hitbox.getPointCount());

		for (int i = 0; i < hitbox.getPointCount(); i++) {
			sf::Vector2f p1 = hitbox.getTransform().transformPoint(hitbox.getPoint(i));
			sf::Vector2f p2 = hitbox.getTransform().transformPoint(hitbox.getPoint(i + 1 == hitbox.getPointCount() ? 0 : i + 1));

			sf::Vector2f normal = sf::Vector2f(-p1.y + p2.y, p1.x - p2.x);

			normals.push_back(normal);
		}

		return normals;
	}

	std::pair<float, float> getMinMax(const sf::Vector2f& normal) {

		sf::Vector2f center = hitbox.getPosition();

		float minProj = std::numeric_limits<float>::max();
		float maxProj = std::numeric_limits<float>::min();

		for (int i = 0; i < hitbox.getPointCount(); i++) {
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
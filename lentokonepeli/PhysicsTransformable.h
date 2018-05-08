#pragma once

#include <SFML\Graphics.hpp>

class PhysicsTransformable : public sf::Transformable {
public:
	PhysicsTransformable(sf::Vector2f pos, float rot, sf::Vector2f velocity_, float drag_, bool gravity_) :drag{ drag_ }, gravity{gravity_ } { PhysicsTransformable(pos, rot, velocity_); }
	PhysicsTransformable(sf::Vector2f pos, float rot, sf::Vector2f velocity_, bool constantVelocity_) : constantVelocity{ constantVelocity_ } { PhysicsTransformable(pos, rot, velocity_); }
	PhysicsTransformable(sf::Vector2f pos, float rot, sf::Vector2f velocity_ = sf::Vector2f(0,0)) { velocity = velocity_; setPosition(pos); setRotation(rot); }
	PhysicsTransformable() {}

	sf::Vector2f acceleration;
	sf::Vector2f velocity;
	sf::Vector2f forceOnSelf;
	float angularVelocity = 0;
	float drag = 0;
	bool gravity = false;
	bool constantVelocity = false;

	void updateValues(PhysicsTransformable&);

	PhysicsTransformable operator*(float);
	PhysicsTransformable operator+(PhysicsTransformable&);
	sf::Vector2f getRotationVector();

	static PhysicsTransformable lerp(PhysicsTransformable& start, PhysicsTransformable& end, float alpha);
	static float lerpAngle(float start, float end, float alpha);
};
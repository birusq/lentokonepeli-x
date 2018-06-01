#include "PhysicsTransformable.h"
#include <cmath>
#include <THOR\Math.hpp>


PhysicsTransformable PhysicsTransformable::operator*(const float b)
{
	PhysicsTransformable a(*this);
	a.setPosition(a.getPosition() * b);
	a.setRotation(a.getRotation() * b);
	a.velocity *= b;
	return a;
}

PhysicsTransformable PhysicsTransformable::operator+(PhysicsTransformable& b)
{
	PhysicsTransformable a(*this);
	a.move(b.getPosition());
	a.rotate(b.getRotation());
	a.velocity += b.velocity;
	return a;
}

sf::Vector2f PhysicsTransformable::getRotationVector() {
	float x = sinf(thor::toRadian(getRotation()));
	float y = -cosf(thor::toRadian(getRotation()));
	return sf::Vector2f(x,y);
}

void PhysicsTransformable::setToRest() {
	velocity = sf::Vector2f();
	acceleration = sf::Vector2f();
	forceOnSelf = sf::Vector2f();
	angularVelocity = 0.0F;
}

void PhysicsTransformable::updateValues(PhysicsTransformable& newPT) {
	setPosition(newPT.getPosition());
	setRotation(newPT.getRotation());
	acceleration = newPT.acceleration;
	velocity = newPT.velocity;
	forceOnSelf = newPT.forceOnSelf;
	drag = newPT.drag;
	gravity = newPT.gravity;
	constantVelocity = newPT.constantVelocity;
}

PhysicsTransformable PhysicsTransformable::lerp(PhysicsTransformable& start, PhysicsTransformable& end, float alpha) {
	PhysicsTransformable res(start);
	float resRotation = lerpAngle(start.getRotation(), end.getRotation(), alpha);

	res = start * (1.0F - alpha) + end * alpha;

	res.setRotation(resRotation);

	return res;
}

float PhysicsTransformable::lerpAngle(float start, float end, float alpha) {
	float difference = abs(end - start);
	if (difference > 180)
	{
		if (end > start)
		{
			start += 360;
		}
		else
		{
			end += 360;
		}
	}
	float value = (start + ((end - start) * alpha));

	return value;
}

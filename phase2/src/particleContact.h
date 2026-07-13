#pragma once

using namespace std;
using namespace glm;

#include "object.h"

class ParticleContact
{
	void resolveVel(float deltaTime)
	{
		float separatingSpeed = getSeparatingSpeed();
		if(separatingSpeed > 0)
			return;

		float actualSepSpeed = separatingSpeed * -restitution;
		float deltaSpeed = actualSepSpeed - separatingSpeed;

		float totalMass = (float)(1 / particles[0]->mass);
		if (particles[1])
			totalMass += (float)(1 / particles[1]->mass);
		if(totalMass <= 0)
			return;

		float impulseMag = deltaSpeed / totalMass;
		vec3 impulse = contactNormal * impulseMag;

		vec3 velA = impulse * (float)(1 / particles[0]->mass);
		particles[0]->velocity += velA;
		if (particles[1])
		{
			vec3 velB = impulse * (float)(-1 / particles[1]->mass);
			particles[1]->velocity += velB;
		}
	}

	void resolveIntersection(float deltaTime)
	{
		if (depth <= 0)
			return;

		float totalMass = (float)(1 / particles[0]->mass);
		if (particles[1])
			totalMass += (float)(1 / particles[1]->mass);

		if (totalMass <= 0)
			return;

		float totalMovePerMass = depth / totalMass;
		vec3 movePerMass = contactNormal * totalMovePerMass;

		vec3 posA = movePerMass * (float)(1 / particles[0]->mass);
		particles[0]->position += posA;
		if (particles[1])
		{
			vec3 posB = movePerMass * (float)(-1 / particles[1]->mass);
			particles[1]->position += posB;
		}

		depth = 0;
	}

	public:
		float depth;
		Object* particles[2];
		float restitution;
		vec3 contactNormal;

		void resolve(float deltaTime)
		{
			resolveVel(deltaTime);
			resolveIntersection(deltaTime);
		}

		float getSeparatingSpeed()
		{
			vec3 vel = particles[0]->velocity;
			if (particles[1])
				vel -= particles[1]->velocity; //check if this is neg

			return dot(vel, contactNormal);
		}
};
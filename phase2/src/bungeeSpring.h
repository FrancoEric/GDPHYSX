#pragma once 

using namespace std;
using namespace glm;

#include "object.h"
#include "forceGenerator.h"

/*
* Bungee explanation:
* removed abs from mag - restlength and it only applies force if the mag is more than the restlength
* so it only moves the particle towards the anchor if the slack is under tension 
*/

class BungeeSpring : public ForceGenerator
{
	vec3 anchorPoint;
	float springCont;
	float restLength;

public:
	BungeeSpring(vec3 pos, float constant, float rest)
	{
		anchorPoint = pos;
		springCont = constant;
		restLength = rest;
	}

	void updateForce(Object* obj, float deltaTime) override
	{
		vec3 pos = obj->getPosition();
		vec3 force = pos - anchorPoint;
		float magnitude = length(force);

		if (magnitude <= restLength || magnitude == 0.0f)
			return;

		float springForce = springCont * (magnitude - restLength);

		if (magnitude > 0)
		{
			force = normalize(force);
		}
		else
		{
			force = vec3(0);
		}
		force = -force * springForce;

		obj->addForce(force);
	}
};
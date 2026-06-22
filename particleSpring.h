#pragma once 

using namespace std;
using namespace glm;

#include "object.h"
#include "forceGenerator.h"

class ParticleSpring : public ForceGenerator
{
	Object* anchorObj;
	float springCont;
	float restLength;

public:
	ParticleSpring(Object* obj, float constant, float rest)
	{
		anchorObj = obj;
		springCont = constant;
		restLength = rest;
	}

	void updateForce(Object* obj, float deltaTime) override
	{
		vec3 pos = obj->getPosition();
		vec3 force = pos - anchorObj->getPosition();
		float magnitude = length(force);
		float springForce = -springCont * abs(magnitude - restLength);

		if (magnitude > 0)
		{
			force = normalize(force);
		}
		else
		{
			force = vec3(0);
		}
		force *= springForce;

		obj->addForce(force);
	}
};
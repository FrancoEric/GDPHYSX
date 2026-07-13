#pragma once 

using namespace std;
using namespace glm;

#include "object.h"
#include "forceGenerator.h"

class AnchoredSpring : public ForceGenerator
{
	vec3 anchorPoint;
	float springCont;
	float restLength;

	public:	
		AnchoredSpring(vec3 pos, float constant, float rest)
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
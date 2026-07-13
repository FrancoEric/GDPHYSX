#pragma once 

using namespace std;
using namespace glm;

#include "forceGenerator.h"

class DragForceGenerator : public ForceGenerator
{
	float k1 = 0.74;
	float k2 = 0.57;

	public:
		DragForceGenerator()
		{

		}

		DragForceGenerator(float k1, float k2)
		{
			this->k1 = k1;
			this->k2 = k2;
		}

		void updateForce(Object* obj, float deltaTime) override
		{
			vec3 force = vec3(0);
			vec3 vel = obj->getVel();
			float mag = length(vel);
			if (mag <= 0)
				return;

			float drag = (k1 * mag) + (k2 * mag);
			vec3 dir = normalize(vel);
			obj->addForce(dir * -drag);
		}
};
#pragma once 

using namespace std;
using namespace glm;

#include "object.h"

class ForceGenerator
{
	public:
		virtual void updateForce(Object* obj, float deltaTime)
		{
			obj->addForce(vec3(0));
		}
};
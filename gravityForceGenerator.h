#pragma once

using namespace std;
using namespace glm;

class GravityForceGenerator : public ForceGenerator
{
	vec3 gravity;

	public:
		GravityForceGenerator(const vec3 gravity)
		{
			this->gravity = gravity;
		}

		void updateForce(Object* obj, float deltaTime) override
		{
			if (obj->mass <= 0)
				return;

			vec3 force = gravity * obj->mass;
			obj->addForce(force);
		}
};
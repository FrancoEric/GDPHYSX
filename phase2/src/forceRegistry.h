#pragma once

using namespace std;
using namespace glm;

#include "forceGenerator.h"

class ForceRegistry
{
	protected:
		struct ParticleForceRegistry
		{
			Object* obj;
			ForceGenerator* generator;
		};

		vector<ParticleForceRegistry> registry;

	public:
		void add(Object* obj, ForceGenerator* generator)
		{
			ParticleForceRegistry newStruct;
			newStruct.obj = obj;
			newStruct.generator = generator;

			registry.push_back(newStruct);
		}

		void remove(Object* obj, ForceGenerator* generator)
		{
			registry.erase(std::remove_if(registry.begin(), registry.end(), [obj, generator](ParticleForceRegistry reg) {
				if (reg.obj == obj && reg.generator == generator)
				{
					return true;
				}
				return false;
				}), registry.end());
		}

		void clear()
		{
			registry.clear();
		}

		void updateForces(float deltaTime)
		{
			for (ParticleForceRegistry pair : registry)
			{
				pair.generator->updateForce(pair.obj, deltaTime);
			}
		}
};
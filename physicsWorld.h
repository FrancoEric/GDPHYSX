#pragma once 

using namespace std;
using namespace glm;

#include "object.h"

class PhysicsWorld
{
	public:
		vector<Object*> particles;

		void addParticle(Object* particle)
		{
			particles.push_back(particle);
		}

		void update(float deltaTime)
		{
			UpdateParticleList();

			for (Object* particle : particles)
			{
				particle->updateParticle(deltaTime);
			}
		}

	private:
		void UpdateParticleList()
		{
			particles.erase(std::remove_if(particles.begin(), particles.end(), [](Object* particle) {
				if (particle->isDestroyed)
				{
					delete particle;

					//cout << "particle destroyed" << endl;
					return true;
				}
				return false;
			}), particles.end());
		}
};
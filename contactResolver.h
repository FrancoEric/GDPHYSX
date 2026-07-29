#pragma once

using namespace std;
using namespace glm;

#include "particleContact.h"

class ContactResolver
{
	unsigned currentIterations = 0;

	public:
		unsigned maxIterations;

		ContactResolver(unsigned maxIterations) : maxIterations(maxIterations) {}

		void resolveContacts(vector<ParticleContact*> contacts, float deltaTime)
		{
			currentIterations = 0;

			while (currentIterations < maxIterations)
			{
                float minClosing = 100000000.0f;
                ParticleContact* chosen = nullptr;

                for (ParticleContact* contact : contacts)
                {
                    float sepSpeed = contact->getSeparatingSpeed();

                    if ((sepSpeed < 0 || contact->depth > 0) && sepSpeed < minClosing)
                    {
                        minClosing = sepSpeed;
                        chosen = contact;
                    }
                }

                if (!chosen)
                    break;

                //std::cout << "Before: "
                //    << chosen->depth << " "
                //    << chosen->getSeparatingSpeed() << std::endl;

                chosen->resolve(deltaTime);
				currentIterations++;

                //std::cout << "After : "
                //    << chosen->depth << " "
                //    << chosen->getSeparatingSpeed() << std::endl;
			}
		}
};
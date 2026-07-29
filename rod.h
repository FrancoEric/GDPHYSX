#pragma once

using namespace std;
using namespace glm;

#include "particleLink.h"
#include "lineDrawable.h"

class Rod : public ParticleLink, public lineDrawable
{
	float restitution = 0; //always 0

	public:
		float length = 1;

		ParticleContact* getContact() override
		{
			float currentLen = currentLength();
			if(currentLen == length)
				return nullptr;

			ParticleContact* contact = new ParticleContact();
			contact->particles[0] = particles[0];
			contact->particles[1] = particles[1];

			vec3 dir = particles[1]->position - particles[0]->position;
			dir = normalize(dir);

			if (currentLen > length)
			{
				contact->contactNormal = dir;
				contact->depth = currentLen - length;
			}
			else
			{
				contact->contactNormal = -dir;
				contact->depth = length - currentLen;
			}

			contact->restitution = restitution;

			return contact;
		}

		void getLineData() override
		{
			pos1 = particles[0]->getPosition();
			pos2 = particles[1]->getPosition();
			color = vec3(1);
		}
};	
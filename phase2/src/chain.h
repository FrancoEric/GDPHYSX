#pragma once

using namespace std;
using namespace glm;

#include "particleLink.h"

/*Chain explanation:
* does nothing when slacked 
* makes contact when strecthed with some bounce 
*/

class Chain : public ParticleLink
{
public:
	float length = 1;
	float restitution = 0.1; //for some bounce 

	ParticleContact* getContact() override
	{
		float currentLen = currentLength();
		if (currentLen <= length)
			return nullptr;

		ParticleContact* contact = new ParticleContact();
		contact->particles[0] = particles[0];
		contact->particles[1] = particles[1];

		vec3 dir = particles[1]->position - particles[0]->position;
		dir = normalize(dir);

		contact->contactNormal = dir;
		contact->depth = currentLen - length;

		contact->restitution = restitution;

		return contact;
	}
};
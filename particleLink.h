#pragma once

using namespace std;
using namespace glm;

#include "particleContact.h"

class ParticleLink
{
	protected:
		float currentLength()
		{
			vec3 len = particles[0]->position - particles[1]->position;
			return length(len);
		}

	public:	
		Object* particles[2];
	
		virtual ParticleContact* getContact() 
		{
			return nullptr;
		}
};
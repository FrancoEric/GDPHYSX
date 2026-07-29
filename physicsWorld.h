#pragma once 

using namespace std;
using namespace glm;

#include "forceRegistry.h"
#include "gravityForceGenerator.h"
#include "dragForceGenerator.h"
#include "contactResolver.h"
#include "particleLink.h"

class PhysicsWorld
{
	public:
		vector<Object*> particles;
		vector<ParticleContact*> contacts;
		vector<ParticleLink*> links;

		ForceRegistry forceRegistry;
		GravityForceGenerator gravity = GravityForceGenerator(vec3(0, 0, 0));
		DragForceGenerator drag = DragForceGenerator(); //defualt ks to 0.1 if not working
		ContactResolver contactResolver = ContactResolver(200);

		bool hasCollisions = true;

		void addContact(Object* obj1, Object* obj2, float restitution, vec3 contactNormal, float depth)
		{
			ParticleContact* newContact = new ParticleContact();
			newContact->particles[0] = obj1;
			newContact->particles[1] = obj2;
			newContact->restitution = restitution;
			newContact->contactNormal = contactNormal;
			newContact->depth = depth;

			contacts.push_back(newContact);
		}

		void applyForceToIndex(int index, vec3 force)
		{
			if(index < 0 || index >= particles.size())
				return;

			particles[index]->addForce(force);
			//cout << "func called";
		}

		void applyForceAtPointToIndex(int index, vec3 force, vec3 point)
		{
			if(index < 0 || index >= particles.size())
				return;

			particles[index]->addForceAtPoint(force, point);
		}

		void addParticle(Object* particle)
		{
			particles.push_back(particle);
			forceRegistry.add(particle, &gravity);
			forceRegistry.add(particle, &drag);
		}

		void addForceGeneratorToIndex(int index, ForceGenerator* generator)
		{
			if(index < 0 || index >= particles.size())
				return;

			forceRegistry.add(particles[index], generator);
		}

		void addLinkToIndex(int index1, int index2, ParticleLink* link)
		{
			if (index1 < 0 || index1 >= particles.size() || index2 < 0 || index2 >= particles.size())
				return;

			link->particles[0] = particles[index1];
			link->particles[1] = particles[index2];

			links.push_back(link);
		}

		void setGravity(vec3 newGravity)
		{
			gravity.setGravity(newGravity);
		}

		void update(float deltaTime)
		{
			UpdateParticleList();

			forceRegistry.updateForces(deltaTime);

			for (Object* particle : particles)
			{
				particle->updateParticle(deltaTime);
			}

			generateContacts();
			
			if (contacts.size() > 0)
			{
				contactResolver.maxIterations = contacts.size() * 4; // scale headroom with contact count
				contactResolver.resolveContacts(contacts, deltaTime);
			}
		}

		void PC02func(bool slow)
		{
			drag.PC02func(slow);
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

		void generateContacts()
		{
			contacts.clear();

			if(hasCollisions)
				getOverlaps();

			for(ParticleLink* link : links)
			{
				ParticleContact* contact = link->getContact();
				if(contact)
					contacts.push_back(contact);
			}
		}

		void getOverlaps()
		{
			for (int i = 0; i < particles.size() - 1; i++)
				for(int j = i + 1; j < particles.size(); j++)
				{
					vec3 dist = particles[i]->position - particles[j]->position;
					float sqrMag = dot(dist, dist);

					float rad = particles[i]->radius + particles[j]->radius;
					float sqrRad = rad * rad;

					if (sqrMag <= sqrRad)
					{
						vec3 dir = normalize(dist);

						float r = sqrRad - sqrMag;
						float depth = sqrt(r);

						float rest = fmin(particles[i]->restitution, particles[j]->restitution);

						addContact(particles[i], particles[j], rest, dir, depth);
					}
				}
		}
};
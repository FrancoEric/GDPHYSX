#pragma once 

using namespace std;
using namespace glm;

#include "object.h"
#include "physicsWorld.h"
#include <random>

class FireworkSpawner
{
	PhysicsWorld* world;
	int fireworkCount;

	//settings
	vec3 spawnPoint = vec3(0);
	float spawnRate = 0.1f;
	float spawnRateTimer = 0;
	int maxMeshIndex = 18; //19 colors
	float minScale = 2;
	float maxScale = 10;
	float mass = 1;
	float gravity = -10;
	float minLifeSpan = 1;
	float maxLifeSpan = 10;

	//direction settings
	float minVel = 3000;
	float maxVel = 5000;
	float Hrange = 0.5f;
	float minV = 0.4;
	float maxV = 1;

	Object* createFirework()
	{
		Object* firework = new Object(spawnPoint, vec3(randomFloat(minScale, maxScale)), mass, gravity, randomInt(0, maxMeshIndex));
		firework->addLifespan(randomFloat(minLifeSpan, maxLifeSpan));

		vec3 dir = vec3(randomFloat(-Hrange, Hrange), randomFloat(minV, maxV), randomFloat(-Hrange, Hrange));
		dir = normalize(dir) * randomFloat(minVel, maxVel);
		firework->addForce(dir);

		return firework;
	}

	int randomInt(int min, int max)
	{
		static random_device rd;
		static mt19937 gen(rd());
		uniform_int_distribution<int> dist(min, max);
		return dist(gen);
	}

	float randomFloat(float min, float max)
	{
		static random_device rd;
		static mt19937 gen(rd());
		uniform_real_distribution<float> dist(min, max);
		return dist(gen);
	}

	vec3 getSpawnPosition()
	{
    	return spawnPoint;
	}

	void setSpawnPosition(vec3 pos)
	{
    	spawnPoint = pos;
	}

	public:
		FireworkSpawner(PhysicsWorld* world, int fireworkCount)
		{
			this->world = world;
			this->fireworkCount = fireworkCount;
		}

		void update(float deltaTime)
		{
			spawnRateTimer += deltaTime;
			if(spawnRateTimer >= spawnRate)
			{
				spawnRateTimer = 0;

				if(world->particles.size() < fireworkCount)
				{
					world->addParticle(createFirework());
				}
			}
		}
};
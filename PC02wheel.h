#pragma once

using namespace std;
using namespace glm;

#include "object.h"
#include "physicsWorld.h"

class PC02wheel
{
	public:
		PhysicsWorld* world;
		Object* pivot;
		Object* ends[5];
		vec3 pushForce;

		void setEndAngles() //apply before loop
		{
			float angleIncrement = 360.0f / 5.0f;
			for (int i = 0; i < 5; i++)
			{
				float angleInRadians = radians(i * angleIncrement);
				ends[i]->position.x = pivot->position.x + cos(angleInRadians);  
				ends[i]->position.y = pivot->position.y + sin(angleInRadians);
			}
		}

		float delay = 3;
		bool pushed = false;
		void initialPush(float deltatime)
		{
			delay -= deltatime;
			if(delay > 0 || pushed)
				return;

			for(Object* end : ends)
			{
				vec3 r = end->position - pivot->position;
				vec3 tangent = normalize(cross(vec3(0, 0, 1), r));

				end->addForce(tangent * length(pushForce));
			}
			pushed = true;
		}

		bool isStopped()
		{
			for(Object* end : ends)
			{
				if(length(end->velocity) > 1.0f)
					return false;
			}
			return true;
		}

		int getWinningIndex()
		{
			float highest = -3000;
			int index = 0;
			for(int i = 0; i < 5; i++)
			{
				if (ends[i]->position.y > highest)
				{
					highest = ends[i]->position.y;
					index = i;
				}
			}

			return index;
		}
};
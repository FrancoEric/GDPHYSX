#pragma once

using namespace std;
using namespace glm;

class Object
{
	vec3 Scale;
	int meshIndex;
	float rotationAngleX;
	float rotationAngleY;
	float rotationAngleZ;

	void updatePos(float deltaTime)
	{
		position = position + velocity * deltaTime + (0.5f) * (acceleration * deltaTime * deltaTime);
	}

	void updateVel(float deltaTime)
	{
		velocity = velocity + acceleration * deltaTime;
	}

	public:
		vec3 position;
		vec3 velocity;
		vec3 acceleration;

		Object(vec3 position, vec3 scale, int meshIndex) 
		{
			this->position = position;
			this->Scale = scale;
			this->meshIndex = meshIndex;
			rotationAngleX = 0;
			rotationAngleY = 0;
			rotationAngleZ = 0;
			velocity = vec3(0, 0, 0);
			acceleration = vec3(0, 0, 0);
		}

		void updateParticle(float deltaTime)
		{
			updatePos(deltaTime);
			updateVel(deltaTime);
		}

		vec3 getPosition() 
		{ 
			return position; 
		}

		int getMeshIndex()
		{
			return meshIndex; 
		}

		mat4 GetTransform()
		{
			mat4 transform = translate(mat4(1.0f), position);
			transform = rotate(transform, rotationAngleX, vec3(1, 0, 0));
			transform = rotate(transform, rotationAngleY, vec3(0, 1, 0));
			transform = rotate(transform, rotationAngleZ, vec3(0, 0, 1));
			transform = scale(transform, Scale);

			return transform;
		}
};
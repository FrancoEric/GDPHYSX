#pragma once

using namespace std;
using namespace glm;

class Object
{
	vec3 Scale;
	vec3 acceleration;
	vec3 force;
	float lifespan;
	float lifespanCounter;
	vec3 accumulatedTorque;

	void updatePos(float deltaTime)
	{
		position = position + velocity * deltaTime + (0.5f) * (acceleration * deltaTime * deltaTime);

		vec3 angleChange = angularVelocity * deltaTime;
		rotation += angleChange;
	}

	void updateVel(float deltaTime)
	{
		float fmass = glm::max(0.000001f, mass);
		acceleration = force / vec3(fmass);

		velocity = velocity + acceleration * deltaTime;
		velocity *= powf(damping, deltaTime);

		float MoI = momentOfInertia();
		angularVelocity += accumulatedTorque * deltaTime * (1/MoI);
		angularVelocity *= powf(angularDamping, deltaTime);
	}

	void updateLifespan(float deltaTime)
	{
		if (lifespan > 0)
		{
			lifespanCounter -= deltaTime;
			if (lifespanCounter <= 0)
			{
				destroyParticle();
			}
		}
	}

	float momentOfInertia()
	{
		return (2.0f / 5.0f) * mass * radius * radius; 
	}

	public:
		vec3 position; //stays in public for rendering
		vec3 targetPos; //useless rn 
		vec3 velocity;
		bool stopMoving;
		bool isDestroyed;
		float damping;
		float mass; //in kgs
		float radius;
		float restitution;
		int meshIndex;
		vec3 rotation = vec3(0); //in euler
		vec3 angularVelocity = vec3(0); 
		float angularDamping = 0.9;
		bool isStatic = false;

		Object(vec3 position, vec3 scale, float mass, float resti, int meshIndex) 
		{
			this->position = position;
			this->Scale = scale;
			this->meshIndex = meshIndex;
			this->mass = mass;
			this->restitution = resti;
			velocity = vec3(0);
			acceleration = vec3(0);
			stopMoving = false;
			isDestroyed = false;
			damping = 1;
			force = vec3(0);
			lifespan = -1; //defaults to no lifespan
			lifespanCounter = lifespan;
			radius = scale.x;
		}

		void updateParticle(float deltaTime)
		{
			if(stopMoving)
				return;

			//cout << "gravity: " << gravity << endl;
			//addForce(vec3(0, gravity, 0));
			//cout << "force: " << force.x << ", " << force.y << ", " << force.z << endl;

			if(!isStatic)
				updatePos(deltaTime);
			updateVel(deltaTime);
			updateLifespan(deltaTime);

			//cout << "position: " << position.x << ", " << position.y << ", " << position.z << endl;
			//cout << "acceleration: " << acceleration.x << ", " << acceleration.y << ", " << acceleration.z << endl;

			resetForce();
		}

		void destroyParticle()
		{
			isDestroyed = true;
			//cout << "particle destroyed" << endl;
		}	

		void addForceAtPoint(vec3 force, vec3 point)
		{
			this->addForce(force);
			accumulatedTorque += cross(point - position, force); //AI filled in the pos, idk if that works 
		}

		void addForce(vec3 newForce)
		{
			force += newForce;
		}

		void resetForce()
		{
			//float fmass = glm::max(0.000001f, mass);
			//acceleration -= force / fmass;

			force = vec3(0);
			acceleration = vec3(0);
			accumulatedTorque = vec3(0);
		}

		void addLifespan(float time)
		{
			lifespan = time;
			lifespanCounter = time;
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
			transform = rotate(transform, rotation.x, vec3(1, 0, 0));
			transform = rotate(transform, rotation.y, vec3(0, 1, 0));
			transform = rotate(transform, rotation.z, vec3(0, 0, 1));
			transform = scale(transform, Scale);

			return transform;
		}

		vec3 getVel()
		{
			return velocity;
		}
};
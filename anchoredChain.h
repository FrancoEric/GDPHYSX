#pragma once 

using namespace std;
using namespace glm;

#include "object.h"
#include "forceGenerator.h"
#include "lineDrawable.h"

class AnchoredChain : public ForceGenerator, public lineDrawable
{
	vec3 anchorPoint;
	float springConst = 2000;
	float damping = 150;
	float restLength;
	Object* obj = nullptr;	

public:
	AnchoredChain(vec3 pos, float rest)
	{
		anchorPoint = pos;
		restLength = rest;
	}

	AnchoredChain(vec3 pos, float rest, Object* obj, vec3 color)
	{
		anchorPoint = pos;
		restLength = rest;
		this->color = color;
		this->obj = obj;
	}

	void getLineData() override
	{
		pos1 = anchorPoint;
		pos2 = obj->getPosition();
	}

	void updateForce(Object* obj, float deltaTime) override
	{
		vec3 pos = obj->getPosition();
		vec3 dir = pos - anchorPoint;
		float magnitude = length(dir);

		if (magnitude == 0.0f)
		{
			return;
		}

		if (magnitude > restLength)
		{
			vec3 direction = dir / magnitude;

			float stretch = magnitude - restLength;

			float speed = dot(obj->velocity, direction);
			float springForce = -springConst * stretch;
			float dampingForce = -damping * speed;

			obj->addForce(direction * (springForce + dampingForce));
		}
	}
};
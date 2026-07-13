#pragma once 

using namespace std;
using namespace glm;

class lineDrawable
{
	public:
		vec3 pos1;
		vec3 pos2;
		vec3 color;

		virtual void getLineData() = 0;
};
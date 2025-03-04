#pragma once
#include <glm/glm.hpp>
#include <vector>

class Vec3Key {
public:
	float mTime;
	glm::vec3 mValue;

	Vec3Key(float time, glm::vec3 value)
	{
		this->mTime = time;
		this->mValue = value;
	}
	Vec3Key(float time)
	{
		this->mTime = time;
		mValue = glm::vec3(0);
	}
	Vec3Key()
	{
		mTime = 0;
		mValue = glm::vec3(0);
	}
};
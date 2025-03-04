#pragma once
#include "Vec3Key.h"
#include "AnimationClip.h"
#include <glm/glm.hpp>
#include <vector>

class Animator
{
	AnimationClip* clip = new AnimationClip;
	bool isPlaying = false;
	float playbackSpeed = 1;
	bool isLooping = false;
	float playbackTime = 0.0f;

	void Update(float dt)
	{
		if (isPlaying)
		{
			playbackTime += dt * playbackSpeed;
			if (playbackTime > clip->duration)
			{
				playbackTime = isLooping ? 0.0f : clip->duration;
				isPlaying = isLooping;
			}
			if (playbackTime < 0.0f)
			{
				playbackTime = isLooping ? clip->duration : 0.0f;
				isPlaying = isLooping;
			}
		}
	}
};
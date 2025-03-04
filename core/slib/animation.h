#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <algorithm>

namespace slib
{
	class Vec3Key
	{
	public:
		float mTime;
		glm::vec3 mValue;

		Vec3Key(float time, glm::vec3 value)
		{
			mTime = time;
			mValue = value;
		}
		Vec3Key(float time)
		{
			mTime = time;
			mValue = glm::vec3(0);
		}
		Vec3Key()
		{
			mTime = 0;
			mValue = glm::vec3(0);
		}
	};

	class AnimationClip
	{
	public:
		float duration;
		std::vector<Vec3Key> positionKeys;
		std::vector<Vec3Key> rotationKeys;
		std::vector<Vec3Key> scaleKeys;
	};

	class Animator
	{
	public:
		AnimationClip* clip = new AnimationClip;
		bool isPlaying = false;
		float playbackSpeed = 1;
		bool isLooping = false;
		float playbackTime = 0;

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
}
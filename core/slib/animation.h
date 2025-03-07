#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace slib
{
	enum EasingMethod
	{
		Lerp,
		EaseInOutSine,
		EaseInOutQuart,
		EaseInOutBack
	};

	class Vec3Key
	{
	public:
		float mTime;
		glm::vec3 mValue;
		int mMethod;

		Vec3Key(float time, glm::vec3 value)
		{
			mTime = time;
			mValue = value;
			mMethod = 0;
		}
		Vec3Key(float time)
		{
			mTime = time;
			mValue = glm::vec3(0);
			mMethod = 0;
		}
		Vec3Key()
		{
			mTime = 0;
			mValue = glm::vec3(0);
			mMethod = 0;
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
		bool isLooping = false;
		float playbackSpeed = 1;
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

		glm::vec3 GetValue(std::vector<Vec3Key> keyFrames, glm::vec3 fallBackValue)
		{
			Vec3Key previous;
			Vec3Key next;
			if (keyFrames.size() >= 2)
			{
				for (int i = 0; i < keyFrames.size(); i++)
				{
					if (keyFrames[i].mTime > playbackTime)
					{
						next = keyFrames[i];
						previous = keyFrames[i - 1];
						break;
					}
				}
				return Easing(previous.mValue, next.mValue, InverseLerp(previous.mTime, next.mTime, playbackTime), slib::EasingMethod(previous.mMethod));
			}
			else
			{
				return fallBackValue;
			}
		}

		glm::vec3 Easing(glm::vec3 a, glm::vec3 b, float t, EasingMethod method)
		{
			switch (method)
			{
			case slib::Lerp:
				return Lerp(a, b, t);
				break;
			case slib::EaseInOutSine:
				return EaseInOutSine(a, b, t);
				break;
			case slib::EaseInOutQuart:
				return EaseInOutQuart(a, b, t);
				break;
			case slib::EaseInOutBack:
				return EaseInOutBack(a, b, t);
				break;
			}
		}

		glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float t)
		{
			return (1 - t) * a + t * b;
		}

		float InverseLerp(float a, float b, float t)
		{
			return (t - a) / (b - a);
		}

		glm::vec3 EaseInOutSine(glm::vec3 a, glm::vec3 b, float t)
		{
			return Lerp(a, b,  -(std::cos(3.141592653589793238462643383279502884197169399375105820974944f * t) - 1) / 2);
		}

		glm::vec3 EaseInOutQuart(glm::vec3 a, glm::vec3 b, float t)
		{
			return Lerp(a, b, t < 0.5 ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2);
		}

		glm::vec3 EaseInOutBack(glm::vec3 a, glm::vec3 b, float t)
		{
			float c1 = 1.70158;
			float c2 = c1 * 1.525;

			return Lerp(a, b, t < 0.5
				? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
				: (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2);
		}
	};
}
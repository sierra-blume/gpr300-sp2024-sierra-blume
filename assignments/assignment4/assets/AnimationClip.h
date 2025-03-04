#pragma once
#include "Vec3Key.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <algorithm>

class AnimationClip
{
	float duration;
	std::vector<Vec3Key> positionKeys;
	std::vector<Vec3Key> rotationKeys;
	std::vector<Vec3Key> scaleKeys;
};
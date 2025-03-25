#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace slib
{
	class Joint {
	public:
		glm::mat4 localTransform;
		glm::mat4 globalTransform;
		Joint* parent;
		std::vector<Joint> children;
		unsigned int numChildren;

		void solveFK(Joint* joint)
		{
			if (joint->parent == NULL)
			{
				joint->globalTransform = joint->localTransform;
			}
			else
			{
				joint->globalTransform = joint->parent->globalTransform * joint->localTransform;
			}
			for (Joint child : children)
			{
				solveFK(&child);
			}
		}
	};
}
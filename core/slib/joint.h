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

		Joint() {
			localTransform = glm::mat4(0);
			globalTransform = glm::mat4(0);
			numChildren = 0;
		}

		Joint addChild()
		{
			Joint add = Joint();
			add.parent = this;
			numChildren++;
			children.push_back(add);
		}

		void solveFK(Joint* joint)
		{
			if (joint->parent == nullptr)
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
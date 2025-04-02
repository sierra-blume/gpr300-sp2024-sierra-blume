#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace slib
{
	class JointPose {
	public:
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;

		JointPose() : rotation(glm::quat(1, 0, 0, 0)), translation(0.0f), scale(1.0f) {}
	};

	class Joint {
	public:
		JointPose localPose;
		glm::mat4 globalPose;
		Joint* parent;
		std::vector<Joint*> children;
		unsigned int numChildren;

		void solveFK(Joint* joint)
		{
			if (joint->parent == nullptr)
			{
				ew::Transform globalPose;
				globalPose.position = joint->localPose.translation;
				globalPose.rotation = joint->localPose.rotation;
				globalPose.scale = joint->localPose.scale;

				joint->globalPose = globalPose.modelMatrix();
			}
			else
			{
				ew::Transform localTransform;
				localTransform.position = joint->localPose.translation;
				localTransform.rotation = joint->localPose.rotation;
				localTransform.scale = joint->localPose.scale;
				joint->globalPose = joint->parent->globalPose * localTransform.modelMatrix();
			}
			for (int i = 0; i < joint->children.size(); i++)
			{
				solveFK(joint->children[i]);
			}
		}
	};
}
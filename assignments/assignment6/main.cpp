#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <slib/animation.h>
#include <slib/joint.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void drawAnimatorUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Creating a camera for us to view our model
ew::Camera camera;

ew::Transform monkeyTransform;
ew::CameraController cameraController;

slib::Animator animator;
slib::Joint root;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

int main() {
	GLFWwindow* window = initWindow("Assignment 6", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Making a shader with the shader files in the assets folder
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	//Loading a 3D model for us to render
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10.0f));
	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(0, -2.0, 0);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);  //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;  //Should be updated every frame or in framebufferSizeCallback to keep the aspect ratio correct after resizing the window
	camera.fov = 60.0f;  //Vertical field of view, in degrees

	animator.clip = new slib::AnimationClip();
	animator.clip->duration = 5;
	animator.isPlaying = true;
	animator.isLooping = true;

	//Building the skeleton
	//Root node
	root.children = std::vector<slib::Joint*>();
	root.localPose.translation = glm::vec3(0, 2.0f, 0);
	root.localPose.scale = glm::vec3(2.0f);

	//Head
	slib::Joint head;
	head.parent = &root;
	head.localPose.translation = glm::vec3(0, 2.75f, 0);
	head.localPose.scale = glm::vec3(1.5f);
	root.children.push_back(&head);

	//Right shoulder
	slib::Joint rShoulder;
	rShoulder.parent = &root;
	rShoulder.children = std::vector<slib::Joint*>();
	rShoulder.localPose.translation = glm::vec3(0.75f, 2.0f, 0);
	rShoulder.localPose.scale = glm::vec3(1.5f);
	root.children.push_back(&rShoulder);

	//Right elbow
	slib::Joint rElbow;
	rElbow.parent = &rShoulder;
	rElbow.children = std::vector<slib::Joint*>();
	rElbow.localPose.translation = glm::vec3(1.25f, 2.0f, 0);
	rElbow.localPose.scale = glm::vec3(1.0f);
	rShoulder.children.push_back(&rElbow);

	//Right wrist
	slib::Joint rWrist;
	rWrist.parent = &rElbow;
	rWrist.localPose.translation = glm::vec3(1.5f, 2.0f, 0);
	rWrist.localPose.scale = glm::vec3(0.5f);
	rElbow.children.push_back(&rWrist);

	//Left shoulder
	slib::Joint lShoulder;
	lShoulder.parent = &root;
	lShoulder.children = std::vector<slib::Joint*>();
	lShoulder.localPose.translation = glm::vec3(-0.75f, 2.0f, 0);
	lShoulder.localPose.scale = glm::vec3(1.5f);
	root.children.push_back(&lShoulder);

	//Left elbow
	slib::Joint lElbow;
	lElbow.parent = &lShoulder;
	lElbow.children = std::vector<slib::Joint*>();
	lElbow.localPose.translation = glm::vec3(-1.25f, 2.0f, 0);
	lElbow.localPose.scale = glm::vec3(1.0f);
	lShoulder.children.push_back(&lElbow);

	//Left wrist
	slib::Joint lWrist;
	lWrist.parent = &lElbow;
	lWrist.localPose.translation = glm::vec3(-1.5f, 2.0f, 0);
	lWrist.localPose.scale = glm::vec3(0.5f);
	lElbow.children.push_back(&lWrist);

	//Setting some global OpenGL variables
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);  //Back face culling
	glEnable(GL_DEPTH_TEST);  //Depth testing

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		/*animator.Update(deltaTime);
		monkeyTransform.position = animator.GetValue(animator.clip->positionKeys, glm::vec3(0));
		monkeyTransform.rotation = animator.GetValue(animator.clip->rotationKeys, glm::vec3(0)) / 180.0f * 3.141592653589793238462643383279502884197169399375105820974944f;
		monkeyTransform.scale = animator.GetValue(animator.clip->scaleKeys, glm::vec3(1));*/

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		//Clears backbuffer color and depth values
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind brick texture to texture unit 0
		glBindTextureUnit(0, brickTexture);

		//Set shader uniforms and draw
		shader.use();
		shader.setVec3("_EyePos", camera.position);
		//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
		shader.setInt("_MainTex", 0);
		//transform.modelMatrix() combines translation, rotation, and scale into a 4x4 model matrix
		//shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		//monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();  //Draws monkey model using current shader

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hierarchy");

	/*if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}*/
	ImGui::End();
	//drawAnimatorUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawAnimatorUI()
{
	const char* easingNames[4] = { "Lerp", "Easing In Out Sine", "Easing In Out Quart", "Easing In Out Back" };

	ImGui::Begin("Animator Settings");
	{
		ImGui::Checkbox("Playing", &animator.isPlaying);
		ImGui::Checkbox("Looping", &animator.isLooping);

		ImGui::SliderFloat("Playback Speed", &animator.playbackSpeed, -5.0f, 5.0f);
		ImGui::SliderFloat("Playback Time", &animator.playbackTime, 0.0f, animator.clip->duration);
		ImGui::DragFloat("Duration", &animator.clip->duration);

		int pushID = 0;
		if (ImGui::CollapsingHeader("Position Keys"))
		{
			for (int i = 0; i < animator.clip->positionKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				ImGui::SliderFloat("Time", &animator.clip->positionKeys[i].mTime, 0.0f, animator.clip->duration);
				ImGui::DragFloat3("Value", &animator.clip->positionKeys[i].mValue.x);
				ImGui::Combo("Interpolation Method", &animator.clip->positionKeys[i].mMethod, easingNames, 4);
				ImGui::PopID();
			}
			ImGui::PushID(pushID++);
			if (ImGui::Button("Add Keyframe"))
			{
				if (animator.clip->positionKeys.size() == 0)
				{
					animator.clip->positionKeys.push_back(slib::Vec3Key(0, glm::vec3(0)));
				}
				else
				{
					animator.clip->positionKeys.push_back(slib::Vec3Key(animator.clip->duration, glm::vec3(0)));
				}
			}
			if (ImGui::Button("Remove Keyframe"))
			{
				if (animator.clip->positionKeys.size() == 0) return;
				else
				{
					animator.clip->positionKeys.pop_back();
				}
			}
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Rotation Keys"))
		{
			for (int i = 0; i < animator.clip->rotationKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				ImGui::SliderFloat("Time", &animator.clip->rotationKeys[i].mTime, 0.0f, animator.clip->duration);
				ImGui::DragFloat3("Value", &animator.clip->rotationKeys[i].mValue.x);
				ImGui::Combo("Interpolation Method", &animator.clip->rotationKeys[i].mMethod, easingNames, 4);
				ImGui::PopID();
			}
			ImGui::PushID(pushID++);
			if (ImGui::Button("Add Keyframe"))
			{
				if (animator.clip->rotationKeys.size() == 0)
				{
					animator.clip->rotationKeys.push_back(slib::Vec3Key(0, glm::vec3(0)));
				}
				else
				{
					animator.clip->rotationKeys.push_back(slib::Vec3Key(animator.clip->duration, glm::vec3(0)));
				}
			}
			if (ImGui::Button("Remove Keyframe"))
			{
				if (animator.clip->rotationKeys.size() == 0) return;
				else
				{
					animator.clip->rotationKeys.pop_back();
				}
			}
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Scale Keys"))
		{
			for (int i = 0; i < animator.clip->scaleKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				ImGui::SliderFloat("Time", &animator.clip->scaleKeys[i].mTime, 0.0f, animator.clip->duration);
				ImGui::DragFloat3("Value", &animator.clip->scaleKeys[i].mValue.x);
				ImGui::Combo("Interpolation Method", &animator.clip->scaleKeys[i].mMethod, easingNames, 4);
				ImGui::PopID();
			}
			ImGui::PushID(pushID++);
			if (ImGui::Button("Add Keyframe"))
			{
				if (animator.clip->scaleKeys.size() == 0)
				{
					animator.clip->scaleKeys.push_back(slib::Vec3Key(0, glm::vec3(1)));
				}
				else
				{
					animator.clip->scaleKeys.push_back(slib::Vec3Key(animator.clip->duration, glm::vec3(1)));
				}
			}
			if (ImGui::Button("Remove Keyframe"))
			{
				if (animator.clip->scaleKeys.size() == 0) return;
				else
				{
					animator.clip->scaleKeys.pop_back();
				}
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}
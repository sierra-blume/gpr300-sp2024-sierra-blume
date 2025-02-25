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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Creating a camera for us to view our model
ew::Camera camera;
ew::CameraController cameraController;

unsigned int depthMap;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//ImGui Stuff
glm::vec3 light;
ew::Transform monkeyTransform;
float bias;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Setting some global OpenGL variables
	glEnable(GL_DEPTH_TEST);  //Depth testing

	//Making a shader with the shader files in the assets folder
	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader depthShader = ew::Shader("assets/depth.vert", "assets/depth.frag");
	//Loading a 3D model for us to render
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10.0f));
	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(0, -2.0, 0);


	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	GLuint tileTexture = ew::loadTexture("assets/Tiles.png");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);  //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;  //Should be updated every frame or in framebufferSizeCallback to keep the aspect ratio correct after resizing the window
	camera.fov = 60.0f;  //Vertical field of view, in degrees

	//Buffer to hold the depth map, which is the depth texture as rendered from the light's perspective
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	//2D texture to be used as framebuffer's depth buffer
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//Attach the generated depth texture as the framebuffer's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//Only need depth info from the lights pov so don't need a color buffer, but opengl needs one for the framebuffer to be complete
	//So, we specify that we aren't going to read or write to it
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	light = glm::vec3(0.25f, 3.0f, -0.5f);
	bias = 0.005f;

	//Orthogragphic projection matrix for the directional light source
	float near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		//Create a view matrix to transform each object so they're visible from the light's pov
		glm::mat4 lightView = glm::lookAt(light, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		//Render scene from light's pov
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);

		depthShader.setMat4("model", planeTransform.modelMatrix());
		planeMesh.draw();

		//Rotate monkey model around Y axis
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		depthShader.setMat4("model", monkeyTransform.modelMatrix());
		monkeyModel.draw();

		glCullFace(GL_BACK);

		//Reset viewport
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		//RENDER
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		//Clears backbuffer color and depth values
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Binding textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tileTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		//Set shader uniforms and draw
		litShader.use();
		litShader.setVec3("_EyePos", camera.position);
		litShader.setVec3("_LightPos", light);
		//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
		litShader.setInt("_MainTex", 1);
		litShader.setInt("_ShadowMap", 2);
		//transform.modelMatrix() combines translation, rotation, and scale into a 4x4 model matrix
		litShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		litShader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);
		litShader.setFloat("_Material.Ka", material.Ka);
		litShader.setFloat("_Material.Kd", material.Kd);
		litShader.setFloat("_Material.Ks", material.Ks);
		litShader.setFloat("_Material.Shininess", material.Shininess);
		litShader.setFloat("_Bias", bias);

		litShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		litShader.setMat4("_Model", monkeyTransform.modelMatrix());
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

	ImGui::Begin("Settings");
	if (ImGui::CollapsingHeader("Directional Light"))
	{
		//MAKE VARIABLES FOR DIRECTIONAL LIGHT//
		ImGui::SliderFloat("Light X", &light.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Light Y", &light.y, 0.0f, 10.0f);
		ImGui::SliderFloat("Light Z", &light.z, -1.0f, 1.0f);
	}
	ImGui::SliderFloat("Monkey X", &monkeyTransform.position.x, -5.0f, 5.0f);
	ImGui::SliderFloat("Monkey Y", &monkeyTransform.position.y, -5.0f, 5.0f);
	ImGui::SliderFloat("Monkey Z", &monkeyTransform.position.z, -5.0f, 5.0f);

	ImGui::SliderFloat("Bias", &bias, 0.0f, 0.5f);

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	ImGui::End();

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)depthMap,  windowSize, ImVec2(0, 1), ImVec2(1,0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
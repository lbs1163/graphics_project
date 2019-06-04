#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool useToon = false;

int main(void) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	Shader toonShader("./src/toon.vert", "./src/toon.frag");
	Shader outlineShader("./src/outline.vert", "./src/outline.frag");
	Model ourModel("./resources/models/Pirates/source/Ship.fbx");
	glActiveTexture(GL_TEXTURE31);
	unsigned int toonTexture = loadTexture("./resources/toon_texture.png");
	glBindTexture(GL_TEXTURE_2D, toonTexture);

	toonShader.use();
	toonShader.setFloat("material.shininess", 32.0f);
	toonShader.setInt("toonTexture", 31);
	
	toonShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	toonShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	toonShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

	toonShader.setVec3("pointLights[0].position", 5.0f, 5.0f, 5.0f);
	toonShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	toonShader.setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
	toonShader.setFloat("pointLights[0].constant", 1.0f);
	toonShader.setFloat("pointLights[0].linear", 0.0014f);
	toonShader.setFloat("pointLights[0].quadratic", 0.000007f);

	toonShader.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
	toonShader.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	toonShader.setVec3("spotLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));
	toonShader.setFloat("spotLight.constant", 1.0f);
	toonShader.setFloat("spotLight.linear", 0.09f);
	toonShader.setFloat("spotLight.quadratic", 0.032f);
	toonShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(22.5f)));
	toonShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(25.0f)));

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		toonShader.use();
		toonShader.setVec3("viewPos", camera.Position);
		toonShader.setVec3("spotLight.position", camera.Position);
		toonShader.setVec3("spotLight.direction", camera.Front);
		toonShader.setBool("useToon", useToon);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		toonShader.setMat4("projection", projection);
		toonShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
		toonShader.setMat4("model", model);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		ourModel.Draw(toonShader);

		outlineShader.use();
		float scale = 1.1f;
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		outlineShader.setMat4("projection", projection);
		outlineShader.setMat4("view", view);
		outlineShader.setMat4("model", model);
		
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		ourModel.Draw(outlineShader);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll((float)yoffset);
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		useToon = !useToon;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
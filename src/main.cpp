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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	Shader toonShader("./src/toon.vert", "./src/toon.frag");
	Model ourModel("./resources/models/pikachu/Pikachu.obj");

	toonShader.use();
	toonShader.setFloat("material.shininess", 32.0f);

	toonShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	toonShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	toonShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

	toonShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	toonShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	toonShader.setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
	toonShader.setFloat("pointLights[0].constant", 1.0f);
	toonShader.setFloat("pointLights[0].linear", 0.09f);
	toonShader.setFloat("pointLights[0].quadratic", 0.032f);

	toonShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	toonShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	toonShader.setVec3("pointLights[1].specular", 0.0f, 0.0f, 0.0f);
	toonShader.setFloat("pointLights[1].constant", 1.0f);
	toonShader.setFloat("pointLights[1].linear", 0.09f);
	toonShader.setFloat("pointLights[1].quadratic", 0.032f);

	toonShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	toonShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	toonShader.setVec3("pointLights[2].specular", 0.0f, 0.0f, 0.0f);
	toonShader.setFloat("pointLights[2].constant", 1.0f);
	toonShader.setFloat("pointLights[2].linear", 0.09f);
	toonShader.setFloat("pointLights[2].quadratic", 0.032f);

	toonShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	toonShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	toonShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	toonShader.setVec3("pointLights[3].specular", 0.0f, 0.0f, 0.0f);
	toonShader.setFloat("pointLights[3].constant", 1.0f);
	toonShader.setFloat("pointLights[3].linear", 0.09f);
	toonShader.setFloat("pointLights[3].quadratic", 0.032f);

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		toonShader.use();
		toonShader.setVec3("viewPos", camera.Position);
		toonShader.setVec3("spotLight.position", camera.Position);
		toonShader.setVec3("spotLight.direction", camera.Front);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		toonShader.setMat4("projection", projection);
		toonShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		toonShader.setMat4("model", model);

		ourModel.Draw(toonShader);

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
}
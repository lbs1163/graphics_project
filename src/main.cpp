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

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 5.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool useToon = true;
bool keyDown = false;

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,		
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	Shader toonShader("./src/toon.vert", "./src/toon.frag");
	Shader outlineShader("./src/outline.vert", "./src/outline.frag", "./src/outline.geom");
	Shader coverShader("./src/cover.vert", "./src/cover.frag");
	Model street("./resources/models/street/Alley.obj");
	Model spiderman("./resources/models/spiderman/spiderman.obj");

	glActiveTexture(GL_TEXTURE31);
	unsigned int toonTexture = loadTexture("./resources/toon_texture.png");
	glBindTexture(GL_TEXTURE_2D, toonTexture);

	glActiveTexture(GL_TEXTURE30);
	unsigned int coverTexture = loadTexture("./resources/models/cover/cover.png");
	glBindTexture(GL_TEXTURE_2D, coverTexture);

	coverShader.use();
	coverShader.setInt("cover", 30);

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

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
		toonShader.setMat4("model", model);

		glStencilMask(0x00);
		street.Draw(toonShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.1f));
		model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		toonShader.setMat4("model", model);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		spiderman.Draw(toonShader);

		outlineShader.use();
		outlineShader.setMat4("projection", projection);
		outlineShader.setMat4("view", view);
		outlineShader.setMat4("model", model);
		
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		spiderman.Draw(outlineShader);
		glStencilMask(0xFF);
		
		glClear(GL_STENCIL_BUFFER_BIT);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 3.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(6.52f, 10.25f, 0.0f));
		coverShader.use();
		coverShader.setMat4("projection", projection);
		coverShader.setMat4("view", view);
		coverShader.setMat4("model", model);
		coverShader.setBool("isBlack", false);
		glActiveTexture(GL_TEXTURE30);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		coverShader.setMat4("projection", glm::mat4(1.0f));
		coverShader.setMat4("view", glm::mat4(1.0f));
		coverShader.setMat4("model", glm::mat4(1.0f));
		coverShader.setBool("isBlack", true);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
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
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !keyDown) {
		useToon = !useToon;
		keyDown = true;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && keyDown)
		keyDown = false;
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
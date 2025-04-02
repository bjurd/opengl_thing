#include <stdio.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <hashmap/map.h>

#include "util.h"
#include "ents.h"

EntityManager_t EntityManager = { 0 };

unsigned int shaderProgram;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float yaw = -90.0f;
float pitch = 0.f;
float lastX = 400, lastY = 300;

vec3 up = { 0, 1.f, 0 };

vec3 cameraPos = { 0, 0, 3.f };
vec3 cameraFront = { 0, 0, -1.f };
vec3 cameraUp = { 0, 1.f, 0 };

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	float cameraSpeed = 2.5f * deltaTime;

	vec3 moveDelta;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm_vec3_scale(cameraFront, cameraSpeed, moveDelta);
		glm_vec3_add(cameraPos, moveDelta, cameraPos);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		glm_vec3_scale(cameraFront, cameraSpeed, moveDelta);
		glm_vec3_sub(cameraPos, moveDelta, cameraPos);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm_vec3_cross(cameraFront, cameraUp, moveDelta);
		glm_vec3_normalize(moveDelta);
		glm_vec3_scale(moveDelta, cameraSpeed, moveDelta);

		glm_vec3_sub(cameraPos, moveDelta, cameraPos);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm_vec3_cross(cameraFront, cameraUp, moveDelta);
		glm_vec3_normalize(moveDelta);
		glm_vec3_scale(moveDelta, cameraSpeed, moveDelta);

		glm_vec3_add(cameraPos, moveDelta, cameraPos);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	vec3 direction;
	direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
	direction[1] = sin(glm_rad(pitch));
	direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
	glm_normalize_to(direction, cameraFront);
}

void TestOnCreation(Entity_t* self)
{
	printf("Created %d\n", self->Index);
}

void TestOnDeletion(Entity_t* self)
{
	printf("Deleted %d\n", self->Index);
}

void TestThink(Entity_t* self, float DeltaTime)
{
	//printf("Thinking %d: %f\n", self->Index, DeltaTime);
}

void TestRender(Entity_t* self, float DeltaTime)
{
	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");

	mat4 trans;
	glm_mat4_identity(trans);
	glm_translate(trans, self->Origin);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)trans);

	//printf("Rendering %d: %f\n", self->Index, DeltaTime);
	ogt_render_entity_basic(self, DeltaTime);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glEnable(GL_DEPTH_TEST);

	Shader_t vertexShader;

	if (!load_shader(GL_VERTEX_SHADER, 1, "../src/shaders/tri.vert", &vertexShader))
		delete_shader(&vertexShader);

	Shader_t fragmentShader;

	if (!load_shader(GL_FRAGMENT_SHADER, 1, "../src/shaders/tri.frag", &fragmentShader))
		delete_shader(&fragmentShader);

	shaderProgram = glCreateProgram();
	attach_shader(&vertexShader, shaderProgram);
	attach_shader(&fragmentShader, shaderProgram);
	glLinkProgram(shaderProgram);

	ogt_init_entity_system(&EntityManager);

	EntityClass_t* Monkey = ogt_register_entity_class("monkey", TestOnCreation, TestOnDeletion, TestThink, TestRender);
	ogt_setup_entity_class_model(Monkey, "../src/models/hahamonkey.obj");

	Entity_t* MokeA = ogt_create_entity_ex(Monkey);
	MokeA->Origin[2] = -30;

	Entity_t* MokeB = ogt_create_entity_ex(Monkey);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		ogt_think_entities(deltaTime);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 trans;
		glm_mat4_identity(trans);

		vec3 cameraForward;
		glm_vec3_add(cameraPos, cameraFront, cameraForward);

		mat4 view;
		glm_lookat(cameraPos, cameraForward, cameraUp, view);

		mat4 projection;
		glm_perspective(glm_rad(45.f), 800.f / 600.f, .1f, 100.f, projection);

		glUseProgram(shaderProgram);

		unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
		unsigned int projectLoc  = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
		glUniformMatrix4fv(projectLoc, 1, GL_FALSE, (float*)projection);

		unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		glUniform3fv(objectColorLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(lightColorLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(lightPosLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(viewPosLoc, 1, (float*)cameraPos);

		unsigned int texUniformLoc = glGetUniformLocation(shaderProgram, "ourTexture");
		glUniform1i(texUniformLoc, 0);

		ogt_render_entities(deltaTime);

		// glBindVertexArray(VAO);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, Materials[0].TextureID);
		// glDrawArrays(GL_TRIANGLES, 0, VertexCount);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

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

unsigned int ShaderProgram;

int WindowWidth = 800, WindowHeight = 600;

float DeltaTime = 0.0f;
float LastFrame = 0.0f;

float CameraYaw = -90.0f, CameraPitch = 0.f;
float LastMouseX = 400, LastMouseY = 300;

vec3 CameraPos = { 0, 0, 3.f };
vec3 CameraFront = { 0, 0, -1.f };
vec3 CameraUp = { 0, 1.f, 0 };

void OnSizeChange(GLFWwindow* Window, int Width, int Height)
{
	WindowWidth = Width;
	WindowHeight = Height;

	glViewport(0, 0, Width, Height);
}

void ProcessInput(GLFWwindow* Window)
{
	float CameraSpeed = 2.5f * DeltaTime;

	vec3 MoveDelta;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm_vec3_scale(CameraFront, CameraSpeed, MoveDelta);
		glm_vec3_add(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		glm_vec3_scale(CameraFront, CameraSpeed, MoveDelta);
		glm_vec3_sub(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm_vec3_cross(CameraFront, CameraUp, MoveDelta);
		glm_vec3_normalize(MoveDelta);
		glm_vec3_scale(MoveDelta, CameraSpeed, MoveDelta);

		glm_vec3_sub(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm_vec3_cross(CameraFront, CameraUp, MoveDelta);
		glm_vec3_normalize(MoveDelta);
		glm_vec3_scale(MoveDelta, CameraSpeed, MoveDelta);

		glm_vec3_add(CameraPos, MoveDelta, CameraPos);
	}
}

void OnMouseMove(GLFWwindow* Window, double MouseX, double MouseY)
{
	float OffsetX = MouseX - LastMouseX;
	float OffsetY = LastMouseY - MouseY;
	LastMouseX = MouseX;
	LastMouseY = MouseY;

	const float Sensitivity = 0.1f;
	OffsetX *= Sensitivity;
	OffsetY *= Sensitivity;

	CameraYaw += OffsetX;
	CameraPitch += OffsetY;

	if (CameraPitch > 89.0f) CameraPitch = 89.0f;
	if (CameraPitch < -89.0f) CameraPitch = -89.0f;

	vec3 Direction;
	Direction[0] = cos(glm_rad(CameraYaw)) * cos(glm_rad(CameraPitch));
	Direction[1] = sin(glm_rad(CameraPitch));
	Direction[2] = sin(glm_rad(CameraYaw)) * cos(glm_rad(CameraPitch));

	glm_normalize_to(Direction, CameraFront);
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

void TestRender(Entity_t* self, float DeltaTime, unsigned int ShaderProgram)
{
	unsigned int modelLoc = glGetUniformLocation(ShaderProgram, "model");

	mat4 trans;
	glm_mat4_identity(trans);
	glm_translate(trans, self->Origin);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)trans);

	//printf("Rendering %d: %f\n", self->Index, DeltaTime);
	ogt_render_entity_basic(self, DeltaTime, ShaderProgram);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* Window = glfwCreateWindow(WindowWidth, WindowHeight, "OpenGL Thing", NULL, NULL);

	if (Window == NULL)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		return -1;
	}

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(Window, OnMouseMove);

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(Window, OnSizeChange);
	glEnable(GL_DEPTH_TEST);

	Shader_t VertexShader;

	if (!load_shader(GL_VERTEX_SHADER, 1, "../src/shaders/tri.vert", &VertexShader))
	{
		printf("Failed to load vertex shader!\n");

		delete_shader(&VertexShader);
		glfwTerminate();

		return -1;
	}

	Shader_t FragmentShader;

	if (!load_shader(GL_FRAGMENT_SHADER, 1, "../src/shaders/tri.frag", &FragmentShader))
	{
		printf("Failed to load fragment shader!\n");

		delete_shader(&FragmentShader);
		glfwTerminate();

		return -1;
	}

	ShaderProgram = glCreateProgram();
	attach_shader(&VertexShader, ShaderProgram);
	attach_shader(&FragmentShader, ShaderProgram);
	glLinkProgram(ShaderProgram);

	ogt_init_entity_system(&EntityManager);

	EntityClass_t* Monkey = ogt_register_entity_class("monkey", TestOnCreation, TestOnDeletion, TestThink, TestRender);

	Entity_t* MokeA = ogt_create_entity_ex(Monkey);
	MokeA->Origin[2] = -30;

	Entity_t* MokeB = ogt_create_entity_ex(Monkey);

	ogt_set_entity_model(MokeA, "../src/models/hahamonkey.obj");
	ogt_set_entity_model(MokeB, "../src/models/monkey.obj");

	while (!glfwWindowShouldClose(Window))
	{
		float Time = glfwGetTime();
		DeltaTime = Time - LastFrame;
		LastFrame = Time;

		ProcessInput(Window);

		ogt_think_entities(DeltaTime);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vec3 CameraForward;
		glm_vec3_add(CameraPos, CameraFront, CameraForward);

		mat4 ViewMatrix;
		glm_lookat(CameraPos, CameraForward, CameraUp, ViewMatrix);

		mat4 ProjectionMatrix;
		glm_perspective(glm_rad(45.f), (float)WindowWidth / (float)WindowHeight, .1f, 100.f, ProjectionMatrix);

		glUseProgram(ShaderProgram);

		unsigned int viewLoc  = glGetUniformLocation(ShaderProgram, "view");
		unsigned int projectLoc  = glGetUniformLocation(ShaderProgram, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)ViewMatrix);
		glUniformMatrix4fv(projectLoc, 1, GL_FALSE, (float*)ProjectionMatrix);

		unsigned int objectColorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
		unsigned int lightColorLoc = glGetUniformLocation(ShaderProgram, "lightColor");
		unsigned int lightPosLoc = glGetUniformLocation(ShaderProgram, "lightPos");
		unsigned int viewPosLoc = glGetUniformLocation(ShaderProgram, "viewPos");

		glUniform3fv(objectColorLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(lightColorLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(lightPosLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
		glUniform3fv(viewPosLoc, 1, (float*)CameraPos);

		unsigned int texUniformLoc = glGetUniformLocation(ShaderProgram, "ourTexture");
		glUniform1i(texUniformLoc, 0);

		ogt_render_entities(DeltaTime, ShaderProgram);

		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

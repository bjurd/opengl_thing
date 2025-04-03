#include <stdio.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/types.h>
#include <hashmap/map.h>

#include "globals.h"
#include "util.h"
#include "ents.h"
#include "render.h"

float DeltaTime = 0.0f;
float LastFrame = 0.0f;

float LastMouseX = 400, LastMouseY = 300;

RenderView_t View = { 0 };

void OnSizeChange(GLFWwindow* Window, int Width, int Height)
{
	GlobalVars->WindowWidth = Width;
	GlobalVars->WindowHeight = Height;

	glViewport(0, 0, Width, Height);
}

void ProcessInput(GLFWwindow* Window)
{
	float CameraSpeed = 2.5f;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(Window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		CameraSpeed = 8.f;

	CameraSpeed *= DeltaTime;

	vec3 CameraPos, CameraForward, CameraRight, CameraUp;
	glm_vec3_copy(View.Origin, CameraPos);
	glm_vec3_copy(View.Forward, CameraForward);

	vec3_directionals(CameraForward, CameraRight, CameraUp);

	vec3 MoveDelta;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		glm_vec3_scale(CameraForward, CameraSpeed, MoveDelta);
		glm_vec3_add(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		glm_vec3_scale(CameraForward, CameraSpeed, MoveDelta);
		glm_vec3_sub(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glm_vec3_cross(CameraForward, CameraUp, MoveDelta);
		glm_vec3_normalize(MoveDelta);
		glm_vec3_scale(MoveDelta, CameraSpeed, MoveDelta);

		glm_vec3_sub(CameraPos, MoveDelta, CameraPos);
	}

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		glm_vec3_cross(CameraForward, CameraUp, MoveDelta);
		glm_vec3_normalize(MoveDelta);
		glm_vec3_scale(MoveDelta, CameraSpeed, MoveDelta);

		glm_vec3_add(CameraPos, MoveDelta, CameraPos);
	}

	glm_vec3_copy(CameraPos, View.Origin);
	glm_vec3_copy(CameraForward, View.Forward);
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

	float Yaw, Pitch;
	vec3_to_angles(View.Forward, &Yaw, &Pitch);

	Yaw += OffsetX;
	Pitch += OffsetY;

	if (Pitch > 89.0f) Pitch = 89.0f;
	if (Pitch < -89.0f) Pitch = -89.0f;

	vec3 Right, Up;
	angles_to_vec3(Yaw, Pitch, View.Forward, Right, Up);
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
	//printf("Rendering %d: %f\n", self->Index, DeltaTime);
	ogt_render_entity_basic(self, DeltaTime);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* Window = glfwCreateWindow(800, 600, "OpenGL Thing", NULL, NULL);

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

	ogt_init_globals();
	GlobalVars->WindowWidth = 800;
	GlobalVars->WindowHeight = 600;

	glViewport(0, 0, GlobalVars->WindowWidth, GlobalVars->WindowHeight);
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

	unsigned int ShaderProgram = glCreateProgram();
	attach_shader(&VertexShader, ShaderProgram);
	attach_shader(&FragmentShader, ShaderProgram);
	glLinkProgram(ShaderProgram);

	EntityClass_t* Monkey = ogt_register_entity_class("monkey", TestOnCreation, TestOnDeletion, TestThink, TestRender);

	Entity_t* MokeA = ogt_create_entity_ex(Monkey);
	MokeA->Origin[2] = -15;

	Entity_t* MokeB = ogt_create_entity_ex(Monkey);
	MokeB->Origin[0] = 5;
	MokeB->Origin[2] = -15;

	Entity_t* Gooba = ogt_create_entity_ex(Monkey);
	Gooba->Origin[0] = -5;
	Gooba->Origin[2] = -15;
	Gooba->Angles[1] = 90.f;

	ogt_set_entity_model(MokeA, "../src/models/hahamonkey.obj");
	ogt_set_entity_model(MokeB, "../src/models/monkey.obj");
	ogt_set_entity_model(Gooba, "../src/models/goobas/vertexcol_mix_sphere.obj");

	ogt_setup_view(&View, (vec3){ 0.f, 0.f, 3.f }, (vec3){ 0, 0, -1.f }, 45.f, .1f, 100.f);

	while (!glfwWindowShouldClose(Window))
	{
		float Time = glfwGetTime();
		DeltaTime = Time - LastFrame;
		LastFrame = Time;

		ProcessInput(Window);

		ogt_think_entities(DeltaTime);

		glUseProgram(ShaderProgram);
		ogt_render_view(&View, DeltaTime);

		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

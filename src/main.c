#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/types.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glEnable(GL_DEPTH_TEST);

	// float vertices[] = {
	// 	// positions          // colors           // texture coords
	// 	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	// 	 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	// 	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	// 	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
	// };

	// unsigned int indices[] = {  // note that we start from 0!
	// 	0, 1, 3,   // first triangle
	// 	1, 2, 3    // second triangle
	// };

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("../src/textures/wall.jpg", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		printf("Failed to load texture\n");

	stbi_image_free(data);

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	// glEnableVertexAttribArray(0);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);
	// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	// glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	shader_t vertexShader;

	if (!load_shader(GL_VERTEX_SHADER, 1, "../src/shaders/tri.vert", &vertexShader))
		delete_shader(&vertexShader);

	shader_t fragmentShader;

	if (!load_shader(GL_FRAGMENT_SHADER, 1, "../src/shaders/tri.frag", &fragmentShader))
		delete_shader(&fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	attach_shader(&vertexShader, shaderProgram);
	attach_shader(&fragmentShader, shaderProgram);
	glLinkProgram(shaderProgram);

	// vec3 cameraDir;
	// glm_vec3_sub(cameraPos, cameraTarget, cameraDir);
	// glm_vec3_normalize(cameraDir);

	// vec3 cameraRight;
	// glm_vec3_cross(up, cameraDir, cameraRight);
	// glm_vec3_normalize(cameraRight);

	// vec3 cameraUp;
	// glm_vec3_cross(cameraDir, cameraRight, cameraUp);

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 trans;
		glm_mat4_identity(trans);
		glm_rotate(trans, glm_rad(25.f), (vec3){1.0f, 0.0f, 0.0f});
		//glm_rotate(trans, (float)glfwGetTime(), (vec3){0.0f, 1.0f, 0.0f});

		// mat4 view;
		// glm_mat4_identity(view);
		// glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});

		vec3 cameraForward;
		glm_vec3_add(cameraPos, cameraFront, cameraForward);

		mat4 view;
		glm_lookat(cameraPos, cameraForward, cameraUp, view);

		mat4 projection;
		glm_perspective(glm_rad(45.f), 800.f / 600.f, .1f, 100.f, projection);

		glUseProgram(shaderProgram);

		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
		unsigned int projectLoc  = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)trans);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
		glUniformMatrix4fv(projectLoc, 1, GL_FALSE, (float*)projection);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();

	return 0;
}

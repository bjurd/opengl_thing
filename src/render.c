#include "render.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "globals.h"

void ogt_setup_view(RenderView_t* View, vec3 Origin, vec3 Forward, float FOV, float NearZ, float FarZ)
{
	glm_vec3_copy(Origin, View->Origin);
	glm_vec3_copy(Forward, View->Forward);
	glm_vec3_normalize(View->Forward);

	View->FOV = FOV;
	View->NearZ = NearZ;
	View->FarZ = FarZ;
	View->AspectRatio = 0.f;

	View->RenderEntities = 1;
}

void ogt_render_view(RenderView_t* View, float DeltaTime)
{
	unsigned int ShaderProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &ShaderProgram);

	if (!ShaderProgram)
	{
		printf("Tried to render view with no shader program!\n");
		return;
	}

	if (View->AspectRatio == 0.f)
		View->AspectRatio = (float)GlobalVars->WindowWidth / (float)GlobalVars->WindowHeight;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vec3 Forward, Right, Up;

	float Yaw, Pitch;
	vec3_to_angles(View->Forward, &Yaw, &Pitch);
	angles_to_vec3(Yaw, Pitch, Forward, Right, Up);

	glm_vec3_add(View->Origin, View->Forward, Forward);

	mat4 ViewMatrix;
	glm_lookat(View->Origin, Forward, Up, ViewMatrix);

	mat4 ProjectionMatrix;
	glm_perspective(glm_rad(View->FOV), View->AspectRatio, View->NearZ, View->FarZ, ProjectionMatrix);

	unsigned int ViewLoc  = glGetUniformLocation(ShaderProgram, "view");
	unsigned int ProjectionLoc  = glGetUniformLocation(ShaderProgram, "projection");

	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, (float*)ViewMatrix);
	glUniformMatrix4fv(ProjectionLoc, 1, GL_FALSE, (float*)ProjectionMatrix);

	// TODO: Update this
	unsigned int lightColorLoc = glGetUniformLocation(ShaderProgram, "lightColor");
	unsigned int lightPosLoc = glGetUniformLocation(ShaderProgram, "lightPos");
	unsigned int viewPosLoc = glGetUniformLocation(ShaderProgram, "viewPos");

	glUniform3fv(lightColorLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
	glUniform3fv(lightPosLoc, 1, (float*)(vec3){ 1.f, 1.f, 1.f });
	glUniform3fv(viewPosLoc, 1, (float*)View->Origin);

	unsigned int texUniformLoc = glGetUniformLocation(ShaderProgram, "ourTexture");
	glUniform1i(texUniformLoc, 0);

	if (View->RenderEntities)
		ogt_render_entities(DeltaTime);
}

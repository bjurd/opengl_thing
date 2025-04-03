#ifndef ogt_render
#define ogt_render

#include <cglm/types.h>

typedef struct
{
	vec3 Origin;
	vec3 Forward;

	float FOV;
	float NearZ;
	float FarZ;
	float AspectRatio;

	bool RenderEntities;
} RenderView_t;

void ogt_setup_view(RenderView_t* View, vec3 Origin, vec3 Forward, float FOV, float NearZ, float FarZ);
void ogt_render_view(RenderView_t* View, float DeltaTime);

#endif

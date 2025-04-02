#ifndef ogt_ents
#define ogt_ents

#define MAX_ENTITIES 255

typedef struct Entity_t Entity_t;
typedef void (*ThinkFn)(Entity_t* self, float DeltaTime);
typedef void (*RenderFn)(Entity_t* self, float DeltaTime);

struct Entity_t
{
	bool Valid;
	unsigned int ID;

	unsigned char Health;

	ThinkFn Think;
	RenderFn Render;
};

typedef struct
{
	unsigned int EntIndex;
	Entity_t* Entities[MAX_ENTITIES];
} EntityManager_t;

extern EntityManager_t EntityManager;

void ogt_create_entity(Entity_t* Entity);
void ogt_think_entities(float DeltaTime);
void ogt_render_entities(float DeltaTime);

#endif

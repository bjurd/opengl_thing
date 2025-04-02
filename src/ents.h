#ifndef ogt_ents
#define ogt_ents

#define MAX_ENTITIES 255

typedef struct Entity_t Entity_t;
typedef void (*CreationFn)(Entity_t* self);
typedef void (*DeletionFn)(Entity_t* self);
typedef void (*ThinkFn)(Entity_t* self, float DeltaTime);
typedef void (*RenderFn)(Entity_t* self, float DeltaTime);

struct Entity_t
{
	bool Valid;
	unsigned int Index;

	unsigned char Health;

	CreationFn OnCreation;
	DeletionFn OnDeletion;
	ThinkFn Think;
	RenderFn Render;
};

typedef struct
{
	unsigned int EntIndex;
	Entity_t* Entities[MAX_ENTITIES];

	unsigned int FreeEntIndices[MAX_ENTITIES];
    unsigned int FreeIndexCount;
} EntityManager_t;

extern EntityManager_t EntityManager;

void ogt_init_entity_system(EntityManager_t* Manager);
void ogt_create_entity(Entity_t* Entity);
void ogt_delete_entity(Entity_t* Entity);
void ogt_think_entities(float DeltaTime);
void ogt_render_entities(float DeltaTime);

#endif

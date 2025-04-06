#include "world.h"

static void OnCreation(Entity_t* self)
{

}

static void OnDeletion(Entity_t* self)
{

}

static void PhysicsInit(Entity_t* self)
{

}

static void Think(Entity_t* self, float DeltaTime)
{

}

static void Render(Entity_t* self, float DeltaTime)
{

}

EntityClass_t* ogt_register_ent_world()
{
	EntityCallbacks_t* Callbacks;

	if (!( Callbacks = ogt_init_entity_callbacks()))
		return NULL;

	Callbacks->OnCreation = OnCreation;
	Callbacks->OnDeletion = OnDeletion;
	Callbacks->InitPhysics = PhysicsInit;
	Callbacks->Think = Think;
	Callbacks->Render = Render;

	return ogt_register_entity_class("world", Callbacks);
}

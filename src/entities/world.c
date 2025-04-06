#include "world.h"

#include <ode/ode.h>

#include "../globals.h"

static void OnCreation(Entity_t* self)
{
	ogt_set_entity_model(self, "../src/models/playne.obj");
}

static void OnDeletion(Entity_t* self)
{

}

static void PhysicsInit(Entity_t* self)
{
	self->Geometry = dCreateBox(GlobalVars->PhysicsManager->Space, 15, 1.0, 15);
	dGeomSetBody(self->Geometry, self->Body);
}

static void Think(Entity_t* self, float DeltaTime)
{

}

static void Render(Entity_t* self, float DeltaTime)
{
	ogt_render_entity_basic(self, DeltaTime);
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

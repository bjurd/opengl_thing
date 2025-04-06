#include "monkey.h"

#include "../globals.h"

static void OnCreation(Entity_t* self)
{
	ogt_set_entity_model(self, "../src/models/spongekey.obj");
}

static void OnDeletion(Entity_t* self)
{

}

static void PhysicsInit(Entity_t* self)
{
	self->Body = dBodyCreate(GlobalVars->PhysicsManager->World);
	dBodySetPosition(self->Body, self->Origin[0], self->Origin[1], self->Origin[2]);
	dBodySetLinearVel(self->Body, 0, 0, 0);
	dBodySetAngularVel(self->Body, 0, 0, 0);

	dMass Mass;
	dMassSetBox(&Mass, 1.0, 1.0, 1.0, 1.0);
	dBodySetMass(self->Body, &Mass);

	self->Geometry = dCreateBox(GlobalVars->PhysicsManager->Space, 1.0, 1.0, 1.0);
	dGeomSetBody(self->Geometry, self->Body);
}

static void Think(Entity_t* self, float DeltaTime)
{

}

static void Render(Entity_t* self, float DeltaTime)
{
	ogt_render_entity_basic(self, DeltaTime);
}

EntityClass_t* ogt_register_ent_monkey()
{
	EntityCallbacks_t* Callbacks;

	if (!( Callbacks = ogt_init_entity_callbacks()))
		return NULL;

	Callbacks->OnCreation = OnCreation;
	Callbacks->OnDeletion = OnDeletion;
	Callbacks->InitPhysics = PhysicsInit;
	Callbacks->Think = Think;
	Callbacks->Render = Render;

	return ogt_register_entity_class("monkey", Callbacks);
}

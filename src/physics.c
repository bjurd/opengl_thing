#include <ode/ode.h>

#include "globals.h"

void ogt_init_physics()
{
	dInitODE();

	GlobalVars->PhysicsManager = malloc(sizeof(PhysicsWorld_t));

	if (!GlobalVars->PhysicsManager)
	{
		printf("Failed to allocate for physics!\n");
		return;
	}

	GlobalVars->PhysicsManager->World = dWorldCreate();
	dWorldSetGravity(GlobalVars->PhysicsManager->World, 0, -9.81, 0);

	GlobalVars->PhysicsManager->Space = dHashSpaceCreate(0);
	GlobalVars->PhysicsManager->ContactGroup = dJointGroupCreate(0);
}

static void near_callback(void* data, dGeomID o1, dGeomID o2)
{
	dContact Contact;
	Contact.surface.mode = dContactBounce | dContactSoftERP | dContactSoftCFM;
	Contact.surface.mu = dInfinity;
	Contact.surface.bounce = 0.0;
	Contact.surface.bounce_vel = 0.0;
	Contact.surface.soft_erp = .2;
	Contact.surface.soft_cfm = .001;

	if (dCollide(o1, o2, 1, &Contact.geom, sizeof(dContact)))
	{
		dJointID c = dJointCreateContact(GlobalVars->PhysicsManager->World, GlobalVars->PhysicsManager->ContactGroup, &Contact);
		dJointAttach(c, dGeomGetBody(o1), dGeomGetBody(o2));
	}
}

void ogt_simulate_physics(float DeltaTime)
{
	dSpaceCollide(GlobalVars->PhysicsManager->Space, 0, &near_callback);
	dWorldStep(GlobalVars->PhysicsManager->World, DeltaTime);
	dJointGroupEmpty(GlobalVars->PhysicsManager->ContactGroup);
}

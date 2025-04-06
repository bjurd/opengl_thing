#ifndef ogt_physics
#define ogt_physics

#include <ode/ode.h>

typedef struct
{
	dWorldID World;
	dSpaceID Space;
	dJointGroupID ContactGroup;
} PhysicsWorld_t;

void ogt_init_physics();
void ogt_simulate_physics(float DeltaTime);

#endif

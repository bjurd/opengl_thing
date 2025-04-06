#include "entfactory.h"

#include "entities/world.h"
#include "entities/monkey.h"

#define LOAD_CLASS(Class, Callback) hashmap_set(EntityFactory, Class, strlen(Class), (uintptr_t)Callback())

hashmap* EntityFactory = NULL;

bool ogt_init_entities()
{
	EntityFactory = hashmap_create();

	if (!EntityFactory)
	{
		printf("Failed to create entity factory!\n");

		return 0;
	}

	LOAD_CLASS("world", ogt_register_ent_world);
	LOAD_CLASS("monkey", ogt_register_ent_monkey);

	return 1;
}

#include "globals.h"

#include <stdio.h>

GlobalVars_t* GlobalVars = NULL;

void ogt_init_globals()
{
	GlobalVars = (GlobalVars_t*)malloc(sizeof(GlobalVars_t));

	if (!GlobalVars)
	{
		printf("Failed to allocate for global vars!\n");
		return;
	}

	GlobalVars->WindowWidth = 0;
	GlobalVars->WindowHeight = 0;

	GlobalVars->EntityManager = NULL;

	ogt_init_entity_system();
}

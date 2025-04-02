#ifndef ogt_globals
#define ogt_globals

#include "ents.h"

typedef struct
{
	EntityManager_t* EntityManager;
} GlobalVars_t;

extern GlobalVars_t* GlobalVars;

void ogt_init_globals();

#endif

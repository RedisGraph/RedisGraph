/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include "group.h"
#include "../redismodule.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../execution_plan/ops/op.h"

// Creates a new group
// arguments specify group's key.
Group *NewGroup(SIValue *keys, uint key_count, AR_ExpNode **funcs, uint func_count, Record r) {
	Group *g = rm_malloc(sizeof(Group));
	g->keys = keys;
	g->aggregationFunctions = funcs;
	g->key_count = key_count;
	g->func_count = func_count;
	g->r = (r) ? OpBase_CloneRecord(r) : NULL;
	return g;
}

void FreeGroup(Group *g) {
	if(g == NULL) return;
	if(g->r) Record_FreeEntries(g->r);  // Will be freed by Record owner.
	if(g->keys) {
		for(int i = 0; i < g->key_count; i ++) SIValue_Free(g->keys[i]);
		rm_free(g->keys);
	}

	for(uint i = 0; i < g->func_count; i++) AR_EXP_Free(g->aggregationFunctions[i]);
	rm_free(g->aggregationFunctions);
	rm_free(g);
}


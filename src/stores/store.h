/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __LABEL_STORE_H__
#define __LABEL_STORE_H__

#include "../redismodule.h"
#include "../graph/entities/graph_entity.h"
#include "../util/triemap/triemap.h"

typedef enum {
  STORE_NODE,
  STORE_EDGE,
} LabelStoreType;

typedef struct {
  int id;               /* Internal ID to a matrix within the graph. */
  char *label;
  TrieMap *properties;  /* Keeps track after label schema. */
} LabelStore;

// Creates a new label store.
LabelStore* LabelStore_New(const char *label, int id);

/* Update store schema with given properties. */
void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties);

/* Attach a pointer value to a property key. */
void LabelStore_AssignValue(LabelStore *store, char *property, void *val);

void* LabelStore_RetrieveValue(LabelStore *store, char *property);

/* Free store. */
void LabelStore_Free(LabelStore *store);

#endif /* __LABEL_STORE_H__ */


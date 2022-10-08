/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./index.h"
#include "../graph/graphcontext.h"

// Indexer handels asynchronously index population
// in the system there's a single instance of it, initialized on module init
//
// whenever an index is to be created, e.g. issuing: CREATE INDEX ON :C(v)
// or updating an existing index by either adding or removing fields from it
// the index population is going to be performed asynchronously by the indexer
// internal working thread, this alows for the system to remain functional
// during index population time

// initialize indexer
bool Indexer_Init(void);

// populates index
// adds the task for populating the given index to the indexer
void Indexer_PopulateIndex
(
	GraphContext *gc, // graph to operate on
	Index *idx        // index to populate
);


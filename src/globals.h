/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "graph/graphcontext.h"

// graphs_in_keyspace iterator
typedef struct GraphIterator {
	uint64_t idx;       // current graph index
} GraphIterator;

// initialize global variables
void Globals_Init(void);

// read global variable 'process_is_child'
bool Globals_Get_ProcessIsChild(void);

// set global variable 'process_is_child'
void Globals_Set_ProcessIsChild
(
	bool process_is_child
);

// get direct access to 'graphs_in_keyspace'
GraphContext **Globals_Get_GraphsInKeyspace(void);

// free globals
void Globals_Free(void);

// add graph to global tracker
void Globals_AddGraph
(
	GraphContext *gc  // graph to add
);

// get graph by name
GraphContext Globals_GetGraph
(
	const char *name  // name of graph to retrieve
);

// remove graph from global tracker
void Globals_RemoveGraph
(
	GraphContext *gc  // graph to remove
);

// remove a graph by its name
void Globals_RemoveGraphByName
(
	const char *name  // graph name to remove
);

// initialize iterator over graphs in keyspace
void Globals_ScanGraphs(GraphIterator *it);

// seek iterator to index
void GraphIterator_Seek
(
	GraphIterator *it,  // iterator
	uint64_t idx        // index to seek to
);

// advance iterator
// returns graph object in case iterator isn't depleted
// otherwise returns NULL
GraphContext *GraphIterator_Next
(
	GraphIterator *it  // iterator to advance
);

// clear all tracked graphs
void Globals_ClearGraphs(void);


/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "globals.h"
#include "util/thpool/pools.h"

struct Globals {
	pthread_rwlock_t lock;              // READ/WRITE lock
	bool process_is_child;              // running process is a child process
	CommandCtx **command_ctxs;          // list of CommandCtxs
	GraphContext **graphs_in_keyspace;  // list of graphs in keyspace
};

struct Globals _globals = {0};

// initialize global variables
void Globals_Init(void) {
	// expecting Global_Init to be called only once
	ASSERT(_globals.graphs_in_keyspace == NULL);

	// initialize
	_globals.process_is_child = false;
	_globals.graphs_in_keyspace = array_new(GraphContext*, 1);
	_globals.command_ctxs = rm_calloc(ThreadPools_ThreadCount() + 1,
			sizeof(CommandCtx *));

	int res = pthread_rwlock_init(&_globals.lock, NULL);
	ASSERT(res == 0);
}

// read global variable 'process_is_child'
bool Globals_Get_ProcessIsChild(void) {
	bool process_is_child = false;

	pthread_rwlock_rdlock(&_globals.lock);

	process_is_child = _globals.process_is_child;

	pthread_rwlock_unlock(&_globals.lock);

	return process_is_child;
}

// set global variable 'process_is_child'
void Globals_Set_ProcessIsChild
(
	bool process_is_child
) {
	pthread_rwlock_wrlock(&_globals.lock);

	_globals.process_is_child =	process_is_child;

	pthread_rwlock_unlock(&_globals.lock);
}

// get direct access to 'graphs_in_keyspace'
GraphContext **Globals_Get_GraphsInKeyspace(void) {
	return _globals.graphs_in_keyspace;
}

// add graph to global tracker
void Globals_AddGraph
(
	GraphContext *gc  // graph to add
) {
	ASSERT(gc != NULL);

	// increase ref count regardless if 'gc' is already tracked
	GraphContext_IncreaseRefCount(gc);

	// acuire write lock
	pthread_rwlock_wrlock(&_globals.lock);

	bool registered = false;
	uint n = array_len(_globals.graphs_in_keyspace);
	for(uint i = 0; i < n; i++) {
		if(_globals.graphs_in_keyspace[i] == gc) {
			registered = true;
			break;
		}
	}

	if(registered == false) {
		// append graph
		array_append(_globals.graphs_in_keyspace, gc);
	}

	// release lock
	pthread_rwlock_unlock(&_globals.lock);
}

// remove graph from global tracker
void Globals_RemoveGraph
(
	GraphContext *gc  // graph to remove
) {
	ASSERT(gc != NULL);

	uint64_t i = 0;
	uint64_t n = array_len(_globals.graphs_in_keyspace);

	// no graphs, early return
	if(n == 0) {
		return;
	}

	// acuire read lock
	pthread_rwlock_rdlock(&_globals.lock);

	// search for graph
	for(; i < n; i++) {
		if(_globals.graphs_in_keyspace[i] == gc) {
			break;
		}
	}

	// release lock
	pthread_rwlock_unlock(&_globals.lock);

	if(i != n) {
		// acuire write lock
		pthread_rwlock_wrlock(&_globals.lock);

		// graph located, remove it
		array_del_fast(_globals.graphs_in_keyspace, i);

		// release lock
		pthread_rwlock_unlock(&_globals.lock);
	}
}

// remove a graph by its name
void Globals_RemoveGraphByName
(
	const char *name  // graph name to remove
) {
	ASSERT(name != NULL);

	// acuire read lock
	pthread_rwlock_rdlock(&_globals.lock);

	// search for graph
	uint64_t i = 0;
	uint64_t n = array_len(_globals.graphs_in_keyspace);
	for(; i < n; i++) {
		GraphContext *gc = _globals.graphs_in_keyspace[i];
		if(strcmp(name, GraphContext_GetName(gc)) == 0) {
			break;
		}
	}

	// release lock
	pthread_rwlock_unlock(&_globals.lock);

	if(i != n) {
		// acuire write lock
		pthread_rwlock_wrlock(&_globals.lock);

		// graph located, remove it
		array_del_fast(_globals.graphs_in_keyspace, i);

		// release lock
		pthread_rwlock_unlock(&_globals.lock);
	}
}

// clear all tracked graphs
void Globals_ClearGraphs(void) {
	// acquire write lock
	pthread_rwlock_wrlock(&_globals.lock);

	// clear graph tracking
	array_clear(_globals.graphs_in_keyspace);

	// release lock
	pthread_rwlock_unlock(&_globals.lock);
}

//------------------------------------------------------------------------------
// Command context tracking
//------------------------------------------------------------------------------

// track CommandCtx
void Globals_TrackCommandCtx
(
	CommandCtx *ctx  // CommandCtx to track
) {
	ASSERT(ctx != NULL);
	ASSERT(_globals.command_ctxs != NULL);

	int tid = ThreadPools_GetThreadID();

	// acuire read lock
	pthread_rwlock_rdlock(&_globals.lock);

	ASSERT(_globals.command_ctxs[tid] == NULL);

	// set ctx at the current thread entry
	_globals.command_ctxs[tid] = ctx;

	// release lock
	pthread_rwlock_unlock(&_globals.lock);

	// reset thread memory consumption to 0 (no memory consumed)
	rm_reset_n_alloced();
}

// untrack CommandCtx
void Globals_UntrackCommandCtx
(
	const CommandCtx *ctx  // CommandCtx to untrack
) {
	ASSERT(ctx != NULL);
	ASSERT(_globals.command_ctxs != NULL);

	int tid = ThreadPools_GetThreadID();

	if(_globals.command_ctxs[tid] == NULL) return; // nothing to clean

	// acuire read lock
	pthread_rwlock_rdlock(&_globals.lock);

	ASSERT(_globals.command_ctxs[tid] == ctx);

	// clear ctx at the current thread entry
	// CommandCtx_Free will free it eventually
	_globals.command_ctxs[tid] = NULL;

	// release lock
	pthread_rwlock_unlock(&_globals.lock);
}

// get a copy of all tracked CommandCtxs
// caller must free each CommandCtx and the array
CommandCtx **Globals_GetCommandCtxs(void) {
	ASSERT(_globals.command_ctxs != NULL);

	// make a copy of the command contexts
	uint32_t nthreads = ThreadPools_ThreadCount();
	CommandCtx **commands = array_new(CommandCtx*, nthreads);

	// acquire write lock
	pthread_rwlock_wrlock(&_globals.lock);

	// increase ref count of each CommandCtx
	for(uint32_t i = 0; i < nthreads; i++) {
		CommandCtx *cmd = _globals.command_ctxs[i];
		if(cmd != NULL) {
			 CommandCtx_Incref(cmd);
			 array_append(commands, cmd);
		}
	}

	// release lock
	pthread_rwlock_unlock(&_globals.lock);

	return commands;
}

// free globals
void Globals_Free(void) {
	rm_free(_globals.command_ctxs);
	array_free(_globals.graphs_in_keyspace);
	pthread_rwlock_destroy(&_globals.lock);
}

//------------------------------------------------------------------------------
// graphs in keyspace iterator
//------------------------------------------------------------------------------

// initialize iterator over graphs in keyspace
void Globals_ScanGraphs(GraphIterator *it) {
	ASSERT(it != NULL);
	it->idx = 0;
}

// seek iterator to index
void GraphIterator_Seek
(
	GraphIterator *it,  // iterator
	uint64_t idx        // index to seek to
) {
	ASSERT(it != NULL);
	it->idx = idx;
}

// advance iterator
// returns graph object in case iterator isn't depleted, otherwise returns NULL
GraphContext *GraphIterator_Next
(
	GraphIterator *it  // iterator to advance
) {
	ASSERT(it != NULL);

	GraphContext *gc = NULL;

	pthread_rwlock_rdlock(&_globals.lock);

	// NOTICE:
	// if caller doesn't hold the GIL thoughout the iterator scan
	// it is possible for the scanning thread to miss keys which are deleted
	// consider the iterator is at position 4 and graph at position 0 is deleted
	// in which case array_del_fast will move the last graph (say index 9) to
	// index 0. in this case the iterator will miss that migrated graph
	if(it->idx < array_len(_globals.graphs_in_keyspace)) {
		// prepare next call
		gc = _globals.graphs_in_keyspace[it->idx++];
		GraphContext_IncreaseRefCount(gc);
	}

	pthread_rwlock_unlock(&_globals.lock);

	return gc;
}


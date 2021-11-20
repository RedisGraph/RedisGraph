/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "redismodule.h"

//------------------------------------------------------------------------------
// code development settings
//------------------------------------------------------------------------------

// to turn on Debug for a single file add:
// #define RG_DEBUG
// just before the statement:
// #include "RG.h"

// to turn on Debug globaly, uncomment this line:
// #define RG_DEBUG

//------------------------------------------------------------------------------
// debugging definitions
//------------------------------------------------------------------------------

#undef ASSERT

#ifdef RG_DEBUG

	// assert X is true
	#define ASSERT(X)                                               \
	{                                                               \
		if (!(X))                                                   \
		{                                                           \
			if(RedisModule__Assert != NULL) {                       \
				RedisModule_Assert(X);				                \
			} else {                                                \
				printf ("assert(" #X ") failed: "                   \
				__FILE__ " line %d\n", __LINE__) ;                  \
				/* force crash */                                   \
				char x = *((char*)NULL); /* produce stack trace */  \
				assert(x); /* solves C++ unused var warning */      \
			}                                                       \
		}                                                           \
	}

#else

	// debugging disabled
	#define ASSERT(X)

#endif


// The unused macro should be applied to avoid compiler warnings
// on set but unused variables

#undef UNUSED
#define UNUSED(V) ((void)V)

//------------------------------------------------------------------------------
// OUT OF KEYSPACE GRAPHS HANDELING
//------------------------------------------------------------------------------

// Return number of out-of-keyspace graphs
#define COUNT_GRAPH_OUTOF_KEYSPACE() ({                                \
	ASSERT(graphs_out_of_keyspace != NULL);                            \
                                                                       \
	array_len(graphs_out_of_keyspace);                                 \
})

// Get graph from out-of-keyspace global array
#define GET_GRAPH_OUTOF_KEYSPACE_IDX(i) ({                             \
	ASSERT(graphs_out_of_keyspace != NULL);                            \
                                                                       \
	uint n = COUNT_GRAPH_OUTOF_KEYSPACE();                             \
	ASSERT(i < n);                                                     \
	graphs_out_of_keyspace[i];                                         \
})

// Add graph to out-of-keyspace global array
#define ADD_GRAPH_OUTOF_KEYSPACE(g) {                                  \
	ASSERT(g != NULL);                                                 \
	ASSERT(graphs_out_of_keyspace != NULL);                            \
                                                                       \
	array_append(graphs_out_of_keyspace, g);                           \
}

// Get graph from out-of-keyspace global array
#define GET_GRAPH_OUTOF_KEYSPACE(g) ({                                 \
	ASSERT(g != NULL);                                                 \
	ASSERT(graphs_out_of_keyspace != NULL);                            \
                                                                       \
	uint i = 0;                                                        \
	uint n = array_len(graphs_out_of_keyspace);                        \
	for(; i < n; i++) {                                                \
		if(strcmp(g , graphs_out_of_keyspace[i]->graph_name) == 0) {   \
			break;			                                           \
		}                                                              \
	}                                                                  \
	(i < n) ? graphs_out_of_keyspace[i] : NULL;                        \
})

// Remove graph from out-of-keyspace global array
#define REMOVE_GRAPH_OUTOF_KEYSPACE_IDX(i) ({                          \
	ASSERT(graphs_out_of_keyspace != NULL);                            \
                                                                       \
	uint n = COUNT_GRAPH_OUTOF_KEYSPACE();                             \
	ASSERT(i < n);                                                     \
                                                                       \
	array_del_fast(graphs_out_of_keyspace, i);                         \
})


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

// Mask with most significat bit on 10000...
#define MSB_MASK (1UL << (sizeof(EntityID) * 8 - 1))
// Mask complement 01111...
#define MSB_MASK_CMP ~MSB_MASK
// Set X's most significat bit on.
#define SET_MSB(x) (x) | MSB_MASK
// Clear X's most significat bit on.
#define CLEAR_MSB(x) (x) & MSB_MASK_CMP
// Checks if X represents edge ID.
#define SINGLE_EDGE(x) (x) & MSB_MASK
// Returns edge ID.
#define SINGLE_EDGE_ID(x) CLEAR_MSB(x)


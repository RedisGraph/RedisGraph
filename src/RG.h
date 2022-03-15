/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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
	_Pragma("GCC diagnostic push")                                  \
	_Pragma("GCC diagnostic ignored \"-Wnull-dereference\"")        \
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
	}                                                               \
	_Pragma("GCC diagnostic pop")

#else

	// debugging disabled
	#define ASSERT(X)

#endif


// The unused macro should be applied to avoid compiler warnings
// on set but unused variables

#undef UNUSED
#define UNUSED(V) ((void)V)

// GraphBLAS return code validation
// both GrB_SUCCESS and GrB_NO_VALUE are valid "OK"
// return codes
#define GrB_OK(GrB_method)                                      \
{                                                               \
	    GrB_Info info = GrB_method ;                            \
	    ASSERT ( (info == GrB_SUCCESS || info == GrB_NO_VALUE)) \
}


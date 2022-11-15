/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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

// use likely and unlikely to provide the compiler with branch prediction information
// for example:
// if (likely(x > 0))
//         foo ();
#if defined(__GNUC__)
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

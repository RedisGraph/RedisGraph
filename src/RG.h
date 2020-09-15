#pragma once
#include <assert.h>

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
	#define ASSERT(X)                               \
	{                                               \
		if (!(X))                                   \
		{                                           \
			printf ("assert(" #X ") failed: "       \
			__FILE__ " line %d\n", __LINE__) ;      \
			assert(false);							\
		}                                           \
	}

#else

	// debugging disabled
	#define ASSERT(X)

#endif


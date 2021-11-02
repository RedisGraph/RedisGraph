//------------------------------------------------------------------------------
// LAGraph_Global:  global variables for LAGraph
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "LG_internal.h"

// These are modified by LAGraph_Init and LAGraph_Xinit.

void * (* LAGraph_Malloc_function  ) (size_t)         = malloc ;
void * (* LAGraph_Calloc_function  ) (size_t, size_t) = calloc ;
void * (* LAGraph_Realloc_function ) (void *, size_t) = realloc ;
void   (* LAGraph_Free_function    ) (void *)         = free ;


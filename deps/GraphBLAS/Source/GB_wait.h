//------------------------------------------------------------------------------
// GB_wait.h: definitions for GB_wait
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_WAIT_H
#define GB_WAIT_H

GB_PUBLIC
GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A,
    GB_Context Context
) ;

GB_PUBLIC
GrB_Info GB_wait                // finish all pending computations
(
    GrB_Matrix A,               // matrix with pending computations
    const char *name,           // name of the matrix
    GB_Context Context
) ;

GrB_Info GB_unjumble        // unjumble a matrix
(
    GrB_Matrix A,           // matrix to unjumble
    GB_Context Context
) ;

// true if a matrix has pending tuples
#define GB_PENDING(A) ((A) != NULL && (A)->Pending != NULL)

// true if a matrix is allowed to have pending tuples
#define GB_PENDING_OK(A) (GB_PENDING (A) || !GB_PENDING (A))

// true if a matrix has zombies
#define GB_ZOMBIES(A) ((A) != NULL && (A)->nzombies > 0)

// true if a matrix is allowed to have zombies
#define GB_ZOMBIES_OK(A) (((A) == NULL) || ((A) != NULL && (A)->nzombies >= 0))

// true if a matrix has pending tuples or zombies
#define GB_PENDING_OR_ZOMBIES(A) (GB_PENDING (A) || GB_ZOMBIES (A))

// true if a matrix is jumbled
#define GB_JUMBLED(A) ((A) != NULL && (A)->jumbled)

// true if a matrix is allowed to be jumbled
#define GB_JUMBLED_OK(A) (GB_JUMBLED (A) || !GB_JUMBLED (A))

// true if a matrix has pending tuples, zombies, or is jumbled
#define GB_ANY_PENDING_WORK(A) \
    (GB_PENDING (A) || GB_ZOMBIES (A) || GB_JUMBLED (A))

// wait if condition holds
#define GB_WAIT_IF(condition,A,name)                                    \
{                                                                       \
    if (condition)                                                      \
    {                                                                   \
        GrB_Info info ;                                                 \
        GB_OK (GB_wait ((GrB_Matrix) A, name, Context)) ;               \
    }                                                                   \
}

// do all pending work:  zombies, pending tuples, and unjumble
#define GB_MATRIX_WAIT(A) GB_WAIT_IF (GB_ANY_PENDING_WORK (A), A, GB_STR (A))

// do all pending work if pending tuples; zombies and jumbled are OK
#define GB_MATRIX_WAIT_IF_PENDING(A) GB_WAIT_IF (GB_PENDING (A), A, GB_STR (A))

// delete zombies and assemble any pending tuples; jumbled is O
#define GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES(A)                         \
    GB_WAIT_IF (GB_PENDING_OR_ZOMBIES (A), A, GB_STR (A))

// ensure A is not jumbled
#define GB_MATRIX_WAIT_IF_JUMBLED(A) GB_WAIT_IF (GB_JUMBLED (A), A, GB_STR (A))

#endif


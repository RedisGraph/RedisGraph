//------------------------------------------------------------------------------
// GB_make_shallow.c: force a matrix to have purely shallow components
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_make_shallow.h"

GB_PUBLIC void GB (make_shallow) (GrB_Matrix A)
{
    if (A == NULL) return ;
    A->p_shallow = (A->p != NULL) ;
    A->h_shallow = (A->h != NULL) ;
    A->b_shallow = (A->b != NULL) ;
    A->i_shallow = (A->i != NULL) ;
    A->x_shallow = (A->x != NULL) ;
    #ifdef GB_MEMDUMP
    printf ("remove from memtable: Ap:%p Ah:%p Ab:%p Ai:%p Ax:%p\n",
        A->p, A->h, A->b, A->i, A->x) ;
    #endif
    if (A->p != NULL) GB_Global_memtable_remove (A->p) ;
    if (A->h != NULL) GB_Global_memtable_remove (A->h) ;
    if (A->b != NULL) GB_Global_memtable_remove (A->b) ;
    if (A->i != NULL) GB_Global_memtable_remove (A->i) ;
    if (A->x != NULL) GB_Global_memtable_remove (A->x) ;
}



/* Copyright 2008-2011, Timothy A. Davis, http://suitesparse.com */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <stddef.h>
#define SPOK_INT int64_t
#define SPOK_OK 1
#define SPOK_WARNING 0

#define SPOK_FATAL_M (-1)
#define SPOK_FATAL_N (-2)
#define SPOK_FATAL_NZMAX (-3)
#define SPOK_FATAL_P (-4)
#define SPOK_FATAL_I (-5)

SPOK_INT spok
(
    /* inputs, not modified */
    SPOK_INT m,             /* number of rows */
    SPOK_INT n,             /* number of columns */
    SPOK_INT nzmax,         /* max # of entries */
    SPOK_INT *Ap,           /* size n+1, column pointers */
    SPOK_INT *Ai,           /* size nz = Ap [n], row indices */
    double *Ax,             /* double matrices always have Ax */
    double *Az,             /* imaginary matrices always have Az */
    char *As,               /* logical matrices always have As */

    /* outputs, not defined on input */
    SPOK_INT *p_njumbled,   /* # of jumbled row indices (-1 if not computed) */
    SPOK_INT *p_nzeros      /* number of explicit zeros (-1 if not computed) */
) ;


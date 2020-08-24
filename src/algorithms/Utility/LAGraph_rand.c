//------------------------------------------------------------------------------
// LAGraph_rand: return a random number
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_rand: return a random number, contributed by Tim Davis, Texas A&M.
// Modified from the POSIX-1.2001 example of rand.

// A simple thread-safe random number generator that returns a random number
// between 0 and LAGRAPH_RAND_MAX.  The quality of the random values it
// generates is very low, but this is not important.  This method is used to
// create random test matrices, which must be identical on any operating
// system.

#include "LAGraph_internal.h"

uint64_t LAGraph_rand (uint64_t *seed)
{
   (*seed) = (*seed) * 1103515245 + 12345 ;
   return (((*seed)/65536) % (LAGRAPH_RAND_MAX + 1)) ;
}


#pragma once

// free a block of memory and set the pointer to NULL
#define LAGRAPH_FREE(p)     \
{                           \
    LAGraph_free (p) ;      \
    p = NULL ;              \
}

void LAGraph_free           // wrapper for free
(
    void *p
);

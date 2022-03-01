

#define GB_GETA( aval, ax, p) aval = (T_Z)ax[ ( p )]
#define GB_GETB( bval, bx, p) bval = (T_Z)bx[ ( p )]
#define GB_FLIP(i)             (-(i)-2)
#define GB_IS_FLIPPED(i)       ((i) < 0)
#define GB_IS_ZOMBIE(i)        ((i) < 0)
#define GB_IS_NOT_FLIPPED(i)   ((i) >= 0)
//#define GB_IS_NOT_ZOMBIE(i)    ((i) >= 0)
#define GB_UNFLIP(i)           (((i) < 0) ? GB_FLIP(i) : (i))

//------------------------------------------------------------------------------
// GB_BINARY_SEARCH
//------------------------------------------------------------------------------

// search for integer i in the list X [pleft...pright]; no zombies.
// The list X [pleft ... pright] is in ascending order.  It may have
// duplicates.

#define GB_BINARY_TRIM_SEARCH(i,X,pleft,pright)                             \
{                                                                           \
    /* binary search of X [pleft ... pright] for integer i */               \
    while (pleft < pright)                                                  \
    {                                                                       \
        int64_t pmiddle = (pleft + pright) / 2 ;                            \
        if (X [pmiddle] < i)                                                \
        {                                                                   \
            /* if in the list, it appears in [pmiddle+1..pright] */         \
            pleft = pmiddle + 1 ;                                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* if in the list, it appears in [pleft..pmiddle] */            \
            pright = pmiddle ;                                              \
        }                                                                   \
    }                                                                       \
    /* binary search is narrowed down to a single item */                   \
    /* or it has found the list is empty */                                 \
    /*ASSERT (pleft == pright || pleft == pright + 1) ;*/                   \
}

//------------------------------------------------------------------------------
// GB_reduce_to_column: reduce a matrix to a column using a binary op
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<mask> = accum (w,reduce(A)) where w is n-by-1

#include "GB.h"

GrB_Info GB_reduce_to_column        // w<mask> = accum (w,reduce(A))
(
    GrB_Matrix w,                   // input/output for results, size n-by-1
    const GrB_Matrix mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp reduce,      // reduce operator for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for w, mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_UNINITIALIZED (accum) ;
    // reduce operator already checked in the caller
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;
    RETURN_IF_UNINITIALIZED (desc) ;

    ASSERT_OK (GB_check (w, "w input for reduce_BinaryOp", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (mask, "mask for reduce_BinaryOp", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for reduce_BinaryOp", 0)) ;
    ASSERT_OK (GB_check (reduce, "reduce for reduce_BinaryOp", 0)) ;
    ASSERT_OK (GB_check (A, "A input for reduce_BinaryOp", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (desc, "desc for reduce_BinaryOp", 0)) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    // inputs are n-by-1 columns
    ASSERT (w->ncols == 1) ;
    ASSERT (IMPLIES (mask != NULL, mask->ncols == 1)) ;

    // check domains and dimensions for w<mask> = accum (w,T)
    GrB_Type T_type = reduce->ztype ;
    info = GB_compatible (w->type, w, mask, accum, T_type) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // check types of reduce
    if (reduce->xtype != reduce->ztype || reduce->ytype != reduce->ztype)
    {
        // all 3 types of z = reduce (x,y) must be the same.  reduce must also
        // be associative but there is no way to check this in general.
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "All domains of reduction operator must be identical;\n"
            "operator is: [%s] = %s ([%s],[%s])", reduce->ztype->name,
            reduce->name, reduce->xtype->name, reduce->ytype->name))) ;
    }

    // T = reduce (T,A) must be compatible
    if (!GB_Type_compatible (A->type, reduce->ztype))
    {
        return (ERROR (GrB_DOMAIN_MISMATCH, (LOG,
            "incompatible type for reduction operator z=%s(x,y):\n"
            "input matrix A of type [%s]\n"
            "cannot be typecast to reduction operator of type [%s]",
            reduce->name, A->type->name, reduce->ztype->name))) ;
    }

    // check the dimensions
    int64_t ancols = A->ncols ;
    int64_t anrows = A->nrows ;
    if (A_transpose)
    {
        if (w->nrows != A->ncols)
        {
            return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
                "w=reduce(A'):  length of w is "GBd";\n"
                "it must match the number of columns of A, which is "GBd".",
                w->nrows, A->ncols))) ;
        }
    }
    else
    {
        if (w->nrows != A->nrows)
        {
            return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
                "w=reduce(A):  length of w is "GBd";\n"
                "it must match the number of rows of A, which is "GBd".",
                w->nrows, A->nrows))) ;
        }
    }

    // quick return if an empty mask is complemented
    RETURN_IF_QUICK_MASK (w, C_replace, mask, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (w) ;
    APPLY_PENDING_UPDATES (mask) ;
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // T = reduce (A) or reduce (A')
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    // T->p is calloc'd since it is just size 2
    GB_NEW (&T, T_type, w->nrows, 1, true, false) ;  // small calloc OK
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    // T = reduce_to_column (A) or reduce_to_column (A'), which is T = sum (A')
    // or sum (A), in MATLAB notation, except where where 'sum' is any
    // associative operator.

    // By default, T(i) = op (A (i,:)) is a column whose length is the same as
    // the number of rows of A.  T(i) is the reduction of all entries in the
    // ith row of A.  If A_transpose is true, the T is computed as if A were
    // transposed first, and thus its length is equal to the number of columns
    // of the input matrix A.  The use of A_transpose is the opposite of
    // MATLAB, since sum(A) in MATLAB sums up the columns of A, and sum(A')
    // sums up the rows of A..

    // T is an n-by-1 GrB_Matrix that represents the column.  It is computed
    // as a matrix so it can be passed to GB_accum_mask without typecasting.

    ASSERT (T->nrows == (A_transpose) ? ancols : anrows) ;
    ASSERT (T->ncols == 1) ;
    ASSERT (!PENDING (T)) ; ASSERT (!ZOMBIES (T)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;

    // FUTURE: this function could easily tolerate zombies in A

    //--------------------------------------------------------------------------
    // scalar workspace
    //--------------------------------------------------------------------------

    size_t asize = A->type->size ;
    const int64_t *Ap = A->p ;
    const int64_t *Ai = A->i ;
    const void *Ax = A->x ;

    size_t zsize = reduce->ztype->size ;
    char awork [zsize] ;
    char zwork [zsize] ;
    char wwork [zsize] ;

    //--------------------------------------------------------------------------
    // T = reduce(A) or reduce(A')
    //--------------------------------------------------------------------------

    GB_binary_function freduce = reduce->function ;
    GB_cast_function
        cast_A_to_Z = GB_cast_factory (reduce->ztype->code, A->type->code) ;
    int64_t tnz = 0 ;
    bool nocasting = (A->type == reduce->ztype) ;

    if (A_transpose)
    {

        //----------------------------------------------------------------------
        // T = reduce(A'), where T(j) = reduce (A (:,j))
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // count the number of entries in the result
        //----------------------------------------------------------------------

        for (int64_t j = 0 ; j < ancols ; j++)
        {
            int64_t ajnz = Ap [j+1] - Ap [j] ;
            if (ajnz > 0)
            {
                tnz++ ;
            }
        }

        //----------------------------------------------------------------------
        // allocate T
        //----------------------------------------------------------------------

        double memory = 0 ;
        if (!GB_Matrix_alloc (T, tnz, true, &memory))
        {
            GB_MATRIX_FREE (&T) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
        T->p [0] = 0 ;
        T->p [1] = tnz ;
        int64_t *Ti = T->i ;
        void *Tx = T->x ;

        //----------------------------------------------------------------------
        // sum down each column: T (j) = reduce (A (:,j))
        //----------------------------------------------------------------------

        bool done = false ;

        tnz = 0 ;

        // define the worker for the switch factory
        #define WORKER(type)                                                \
        {                                                                   \
            const type *ax = (type *) Ax ;                                  \
            type *tx = (type *) Tx ;                                        \
            for (int64_t j = 0 ; j < ancols ; j++)                          \
            {                                                               \
                /* w = reduce (A (:,j)) */                                  \
                type w ;                                                    \
                int64_t p = Ap [j] ;                                        \
                int64_t ajnz = Ap [j+1] - p ;                               \
                /* get the first entry in column j */                       \
                if (ajnz > 0)                                               \
                {                                                           \
                    /* w = Ax [p], the first entry in column j */           \
                    w = ax [p] ;                                            \
                    p++ ;                                                   \
                }                                                           \
                /* subsequent entries in column j */                        \
                for ( ; p < Ap [j+1] ; p++)                                 \
                {                                                           \
                    /* w "+=" ax [p] ; */                                   \
                    ADD (w, ax [p]) ;                                       \
                }                                                           \
                if (ajnz > 0)                                               \
                {                                                           \
                    Ti [tnz] = j ;                                          \
                    tx [tnz] = w ;                                          \
                    tnz++ ;                                                 \
                }                                                           \
            }                                                               \
            done = true ;                                                   \
        }

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GB_COMPACT is defined, the switch factory is disabled and all
        // work is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            if (nocasting)
            {
                // controlled by opcode and typecode.  No typecasting is done.
                GB_Opcode opcode = reduce->opcode ;
                GB_Type_code typecode = A->type->code ;
                ASSERT (typecode <= GB_UDT_code) ;
                #include "GB_assoc_template.c"
            }

        #endif

        #undef WORKER

        //----------------------------------------------------------------------
        // generic worker
        //----------------------------------------------------------------------

        if (!done)
        {
            for (int64_t j = 0 ; j < ancols ; j++)
            {
                // zwork = reduce (A (:,j))
                int64_t p = Ap [j] ;
                int64_t ajnz = Ap [j+1] - p ;
                // get the first entry in column j
                if (ajnz > 0)
                {
                    // zwork = (ztype) Ax [p], the first entry in column j
                    cast_A_to_Z (zwork, Ax +(p*asize), zsize) ;
                    p++ ;
                }
                // subsequent entries in column j
                for ( ; p < Ap [j+1] ; p++)
                {
                    // awork = (ztype) Ax [p]
                    cast_A_to_Z (awork, Ax +(p*asize), zsize) ;
                    // wwork = zwork
                    memcpy (wwork, zwork, zsize) ;
                    // zwork = wwork "+" awork
                    freduce (zwork, wwork, awork) ;
                }
                if (ajnz > 0)
                {
                    Ti [tnz] = j ;
                    // Tx [tnz] = zwork ;
                    memcpy (Tx +(tnz*zsize), zwork, zsize) ;
                    tnz++ ;
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // T = reduce(A), where T(i) = reduce (A (i,:))
        //----------------------------------------------------------------------

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

        // ensure Mark is at least of size anrows+1
        // Mark [i] = flag if work [i] is "nonzero"
        bool ok = GB_Mark_alloc (anrows + 1) ;
        int64_t *Mark = GB_thread_local.Mark ;
        int64_t flag = GB_Mark_reset (1, 0) ;

        // ensure Work is at least size (anrows+1) * (zsize+sizeof(int64_t)
        ok = ok && GB_Work_alloc (anrows + 1, zsize + sizeof (int64_t)) ;
        void *work = GB_thread_local.Work ;
        int64_t *wpattern = (int64_t *) (work + (anrows+1) * zsize) ;

        double memory = GBYTES (anrows+1, zsize + 2 * sizeof (int64_t)) ;

        if (!ok)
        {
            GB_MATRIX_FREE (&T) ;
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }

        //----------------------------------------------------------------------
        // sum across each row: work [i] = reduce (A (i,:))
        //----------------------------------------------------------------------

        bool done = false ;

        // define the worker for the switch factory
        #define WORKER(type)                                                \
        {                                                                   \
            const type *ax = (type *) Ax ;                                  \
            type *ww = (type *) work ;                                      \
            int64_t anz = Ap [ancols] ;                                     \
            for (int64_t p = 0 ; p < anz ; p++)                             \
            {                                                               \
                /* get A(i,j) */                                            \
                int64_t i = Ai [p] ;                                        \
                if (Mark [i] != flag)                                       \
                {                                                           \
                    /* first time row i has been seen */                    \
                    ww [i] = ax [p] ;                                       \
                    Mark [i] = flag ;                                       \
                    wpattern [tnz++] = i ;                                  \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* ww [i] "+=" ax [p] */                                \
                    ADD (ww [i], ax [p]) ;                                  \
                }                                                           \
            }                                                               \
            done = true ;                                                   \
        }

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GB_COMPACT is defined, the switch factory is disabled and all
        // work is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            if (nocasting)
            {
                // controlled by opcode and typecode.  No typecasting is done.
                GB_Opcode opcode = reduce->opcode ;
                GB_Type_code typecode = A->type->code ;
                ASSERT (typecode <= GB_UDT_code) ;
                #include "GB_assoc_template.c"
            }

        #endif

        #undef WORKER

        //----------------------------------------------------------------------
        // generic worker
        //----------------------------------------------------------------------

        if (!done)
        {
            int64_t anz = NNZ (A) ;
            for (int64_t p = 0 ; p < anz ; p++)
            {
                // get A(i,j)
                int64_t i = Ai [p] ;
                if (Mark [i] != flag)
                {
                    // work [i] = (ztype) Ax [p]
                    cast_A_to_Z (work +(i*zsize), Ax +(p*asize), zsize) ;
                    Mark [i] = flag ;
                    wpattern [tnz++] = i ;
                }
                else
                {
                    // awork = (ztype) Ax [p]
                    cast_A_to_Z (awork, Ax +(p*asize), zsize) ;
                    // wwork = work [i]
                    memcpy (wwork, work +(i*zsize), zsize) ;
                    // zwork = wwork "+" awork
                    freduce (zwork, wwork, awork) ;
                    // work [i] = zwork
                    memcpy (work +(i*zsize), zwork, zsize) ;
                }
            }
        }

        //----------------------------------------------------------------------
        // clear the Mark array
        //----------------------------------------------------------------------

        GB_Mark_reset (1, 0) ;

        //----------------------------------------------------------------------
        // allocate T
        //----------------------------------------------------------------------

        if (!GB_Matrix_alloc (T, tnz, true, &memory))
        {
            GB_MATRIX_FREE (&T) ;
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
        T->p [0] = 0 ;
        T->p [1] = tnz ;
        int64_t *Ti = T->i ;
        void *Tx = T->x ;

        //----------------------------------------------------------------------
        // sort the pattern of T
        //----------------------------------------------------------------------

        GB_qsort_1 (wpattern, tnz) ;

        //----------------------------------------------------------------------
        // copy the result into T
        //----------------------------------------------------------------------

        for (int64_t p = 0 ; p < tnz ; p++)
        {
            int64_t i = wpattern [p] ;
            Ti [p] = i ;
            // Tx [p] = work [i]
            memcpy (Tx +(p*zsize), work +(i*zsize), zsize) ;
        }
    }

    ASSERT_OK (GB_check (T, "T output for T =reduce (A)", 0)) ;

    //--------------------------------------------------------------------------
    // w<mask> = accum (w,T): accumulate the results into w via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (w, mask, accum, &T, C_replace, Mask_comp)) ;
}


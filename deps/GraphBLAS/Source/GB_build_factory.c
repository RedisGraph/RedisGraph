//------------------------------------------------------------------------------
// GB_build_factory: build a matrix from sorted tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The tuples have been sorted and duplicates marked by GB_builder.  Assemble
// them with a switch factory of built-in workers, or two generic workers.  The
// column pointers C->p have already been computed.  The rest of the build
// process here needs to know nothing at all about the columns.  kwork is NULL
// on input if the original input tuples did not need sorting; in this cask,
// kwork [k] == k is implicitly true.

// iwork holds the row indices of the tuple, and kwork holds the positions in
// X.  The tuples are sorted so that duplicates are adjacent to each other and
// they appear in the order they appeared in the original tuples.  This method
// assembles the duplicates and creates C->x from iwork, kwork, and X.
// iwork is then transplanted into C,  becoming C->i.

// On input, the (i,k,x[k]) tuples are held in two integer arrays, iwork and
// kwork, and an array X of numerical values.  X has not been sorted, nor even
// accessed yet.  It is identical to the original unsorted tuples.  The
// (i,k,X[k]) tuple holds the row index i, the position k, and the value X [k].
// This entry becomes C(i,j) = X [k] in the matrix C, and duplicates are
// assembled via the dup operator.

// The row indices on input are in iwork, and after duplicates are removed,
// iwork is compacted (duplicates removed) and then transplanted directly in
// the C, becoming the row indices C->i.  The symbolic analysis is thus
// consumed by this function, and incorporated into the output matrix C, in
// place.  If this method is split into user-callable symbolic analysis and
// numerical phases, then a copy of iwork should be made, which would then be
// consumed and transplanted into C->i.  Also, kwork (which is read-only by
// thus function) should not be freed.  If these changes were made, then iwork
// and kwork could be used for subsequent builds of C with the same pattern and
// ordering of tuples but with different numerical values.

// On output, kwork is freed and iwork becomes C->i.   Thus iwork_handle and
// kwork_handle in the caller are both set to NULL.

#include "GB.h"

GrB_Info GB_build_factory           // build a matrix
(
    GrB_Matrix C,                   // matrix to build
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **kwork_handle,         // for (i,k) or (j,i,k) tuples
    const void *X,                  // array of values of tuples
    const int64_t len,              // number of tuples and size of kwork
    const int64_t ilen,             // size of iwork array
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to 
                                    // keep the most recent duplicate.
    const GB_Type_code X_code       // GB_Type_code of X array
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    ASSERT (X != NULL) ;
    ASSERT (iwork_handle != NULL) ;
    ASSERT (kwork_handle != NULL) ;
    ASSERT (X_code <= GB_UDT_code) ;

    const int64_t *kwork = *kwork_handle ;      // if NULL, kwork [k] == k
    int64_t *iwork = *iwork_handle ;            // cannot be NULL

    ASSERT (iwork != NULL) ;

    //--------------------------------------------------------------------------
    // allocate the result
    //--------------------------------------------------------------------------

    // get the sizes of each type
    size_t csize = C->type->size ;
    size_t xsize = GB_Type_size (X_code, csize) ;

    // C->x is allocated fresh but iwork is transplanted in C->i, when done.
    // This malloc is typically free since jwork has just been freed.  C->x has
    // size cnz*csize and jwork is size len*sizeof(int64_t).  cnz <= len always
    // holds, and csize <= size(int64_t) holds for all built-in types.
    GB_MALLOC_MEMORY (C->x, C->nzmax, csize) ;
    C->i = NULL ;
    double memory = GBYTES (C->nzmax, csize) ;
    if (C->x == NULL)
    {
        // out of memory
        GB_FREE_MEMORY (*kwork_handle, len, sizeof (int64_t)) ;
        GB_FREE_MEMORY (*iwork_handle, ilen, sizeof (int64_t)) ;
        GB_Matrix_clear ((GrB_Matrix) C) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    void *Cx = C->x ;

    //--------------------------------------------------------------------------
    // assemble the output
    //--------------------------------------------------------------------------

    GB_binary_function fdup ;
    if (dup == NULL)
    {
        // GrB_setElement and GrB_*assign have assigned entries to the matrix
        // and they are being assembed by GB_wait, which passes in NULL for the
        // operator.  The operator is implicitly the SECOND operator, which
        // keeps the most recent duplicates.
        fdup = NULL ;
    }
    else
    {
        // z = fdup (x,y) where x, y, and z are all of the same type
        fdup = dup->function ;
    }

    int64_t cnz = 0 ;

    if (C->type->code == X_code)
    {

        //----------------------------------------------------------------------
        // copy the values, X, into C, no typecasting needed
        //----------------------------------------------------------------------

        // There are 44 common cases of this function for built-in types and
        // 8 associative operators: min, max, plus, times for all types;
        // or, and, xor, eq for boolean.

        // In addition, the FIRST and SECOND operators hard-coded, for another
        // 22 workers, since SECOND is used by GB_wait and since FIRST is
        // useful for keeping the first tuple seen.  It is controlled by the
        // #define, so they don't appear in GB_reduce_to_* where the FIRST and
        // SECOND operators are not needed.

        #define INCLUDE_SECOND_OPERATOR

        bool done = false ;

        // define the worker for the switch factory
        #define WORKER(type)                                                \
        {                                                                   \
            const type *xx = (type *) X ;                                   \
            type *cx = (type *) Cx ;                                        \
            for (int64_t t = 0 ; t < len ; t++)                             \
            {                                                               \
                /* get the (i,k) or (j,i,k) tuple and check if duplicate */ \
                int64_t i = iwork [t] ;                                     \
                int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;           \
                if (i < 0)                                                  \
                {                                                           \
                    /* duplicate entry: Cx [cnz-1] '+=' X [k] */            \
                    ADD (cx [cnz-1], xx [k]) ;                              \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* new entry; save its index and value */               \
                    ASSERT (cnz <= t) ;                                     \
                    cx [cnz] = xx [k] ;                                     \
                    iwork [cnz] = i ;                                       \
                    cnz++ ;                                                 \
                }                                                           \
            }                                                               \
            done = true ;                                                   \
        }

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GBCOMPACT is defined, the switch factory is disabled and all work
        // is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            // controlled by opcode and typecode
            GB_Opcode opcode = (dup == NULL) ?  GB_SECOND_opcode:(dup->opcode) ;
            GB_Type_code typecode = C->type->code ;
            ASSERT (typecode <= GB_UDT_code) ;
            #include "GB_assoc_template.c"

        #endif

        #undef WORKER
        #undef INCLUDE_SECOND_OPERATOR

        //----------------------------------------------------------------------
        // generic worker
        //----------------------------------------------------------------------

        if (!done)
        {

            //------------------------------------------------------------------
            // no typecasting, but use the fdup function pointer and memcpy
            //------------------------------------------------------------------

            // The switch case didn't handle this case.  Any binary operator
            // can be passed to this function, even non-associative built-in
            // operators.  User-defined types are handled here as well, since
            // no typecasting can be done.  This is slower than using
            // hard-coded types and operators.

            // scalar workspace
            char zwork [csize] ;
            size_t size = csize ;       // all types have the same size
            ASSERT (csize == xsize) ;

            for (int64_t t = 0 ; t < len ; t++)
            {
                // get the (i,k) or (j,i,k) tuple and check if duplicate
                int64_t i = iwork [t] ;
                int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
                if (i < 0)
                {
                    // duplicate entry: assemble it with the dup operator
                    if (dup == NULL)
                    {
                        // Cx [cnz-1] = X [k], the implied "SECOND" operator
                        memcpy (Cx +((cnz-1)*size), X +(k*size), size) ;
                    }
                    else
                    {
                        // Cx [cnz-1] '+=' X [k]
                        fdup (zwork, Cx +((cnz-1)*size), X +(k*size)) ;
                        memcpy (Cx +((cnz-1)*size), zwork, size) ;
                    }
                }
                else
                {
                    // new entry; save its index and value
                    ASSERT (cnz <= t) ;
                    // Cx [cnz] = X [k]
                    memcpy (Cx +(cnz*size), X +(k*xsize), size) ;
                    iwork [cnz] = i ;
                    cnz++ ;
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // copy the values X into C, typecasting as needed
        //----------------------------------------------------------------------

        // function to cast X to C
        GB_cast_function cast_X_to_C = GB_cast_factory (C->type->code, X_code) ;

        // GB_wait (which allows dup to be NULL) does not use this part of the
        // code since it doesn't do any typecasting
        ASSERT (dup != NULL && fdup != NULL) ;

        // The type of the X array differs from the type of C and dup, but both
        // types are built-in since user-defined types can't be typecasted.
        ASSERT (X_code != GB_UDT_code) ;
        ASSERT (C->type->code != GB_UDT_code) ;

        // scalar workspace
        char ywork [csize] ;
        char zwork [csize] ;

        for (int64_t t = 0 ; t < len ; t++)
        {
            // get the (i,k) or (j,i,k) tuple and check if it's a duplicate
            int64_t i = iwork [t] ;
            int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
            if (i < 0)
            {
                // duplicate entry: Cx [cnz-1] '+=' X [k]
                // ywork = (cast to C) X [k]
                cast_X_to_C (ywork, X +(k*xsize), xsize) ;
                // zwork = f (Cx [cnz-1], ywork)
                fdup (zwork, Cx +((cnz-1)*csize), ywork) ;
                // Cx [cnz-1] = zwork
                memcpy (Cx +((cnz-1)*csize), zwork, csize) ;
            }
            else
            {
                // new entry; save its index and value
                // Cx [cnz] = (cast to C) X [k]
                cast_X_to_C (Cx +(cnz*csize), X +(k*xsize), xsize) ;
                iwork [cnz] = i ;
                cnz++ ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace 
    //--------------------------------------------------------------------------

    // In the current implementation, kwork is no longer needed.  If this
    // method were to be split into user-callable symbolic and numerical
    // phases, kwork is part of the symbolic analysis and should be kept for
    // subsequent builds with the same I and J but different X.

    GB_FREE_MEMORY (*kwork_handle, len, sizeof (int64_t)) ;
    kwork = NULL ;

    //--------------------------------------------------------------------------
    // transplant iwork into C->i
    //--------------------------------------------------------------------------

    // shrink iwork from size len to size C->nzmax
    if (C->nzmax < ilen)
    {
        // this cannot fail since the size is shrinking.
        bool ok ;
        GB_REALLOC_MEMORY (iwork, C->nzmax, ilen, sizeof (int64_t), &ok) ;
        ASSERT (ok) ;
    }
    C->i = iwork ;
    iwork = NULL ;

    // iwork has been transplanted into C, as C->i, so set the iwork pointer
    // in the caller to NULL.
    *iwork_handle = NULL ;

    return (REPORT_SUCCESS) ;
}


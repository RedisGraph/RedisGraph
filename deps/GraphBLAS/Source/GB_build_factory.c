//------------------------------------------------------------------------------
// GB_build_factory: build a matrix from sorted tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The tuples have been sorted and duplicates marked by GB_builder.  Assemble
// them with a switch factory of built-in workers, or two generic workers.  The
// vector pointers T->p and hyperlist T->h (if hypersparse) have already been
// computed.  This function is agnostic regarding the CSR/CSR format, and even
// hypersparsity.  The rest of the build process here needs to know nothing at
// all about the vectors.  kwork is NULL on input if the original input tuples
// did not need sorting; in this case, kwork [k] == k is implicitly true.

// iwork holds the row indices of the tuple, and kwork holds the positions in
// the array S.  The tuples are sorted so that duplicates are adjacent to each
// other and they appear in the order they appeared in the original tuples.
// This method assembles the duplicates and creates T->x from iwork, kwork, and
// S.  iwork is then transplanted into T, becoming T->i.

// On input, the (i,k,S[k]) tuples are held in two integer arrays, iwork and
// kwork, and an array S of numerical values.  S has not been sorted, nor even
// accessed yet.  It is identical to the original unsorted tuples.  The
// (i,k,S[k]) tuple holds the row index i, the position k, and the value S [k].
// This entry becomes T(i,j) = S [k] in the matrix T, and duplicates are
// assembled via the dup operator.

// The row indices on input are in iwork, and after duplicates are removed,
// iwork is compacted (duplicates removed) and then transplanted directly in
// the T, becoming the row indices T->i.  The symbolic analysis is thus
// consumed by this function, and incorporated into the output matrix T, in
// place.  If this method is split into user-callable symbolic analysis and
// numerical phases, then a copy of iwork should be made, which would then be
// consumed and transplanted into T->i.  Also, kwork (which is read-only by
// thus function) should not be freed.  If these changes were made, then iwork
// and kwork could be used for subsequent builds of T with the same pattern and
// ordering of tuples but with different numerical values.

// On output, kwork is freed and iwork becomes T->i.   Thus iwork_handle and
// kwork_handle in the caller are both set to NULL.

// The time and memory taken by this function is O(t) if t=len is the number
// of tuples.

// PARALLEL: the tuples have already been sorted, and duplicates tagged.  need
// to parallelize the summation of duplicate tuples.  Each unique tuple could
// be done only by the thread the owns it.  It is unlikely that there will be
// many duplicates, but possible.  So consider a parallel reduction.

#include "GB.h"

GrB_Info GB_build_factory           // build a matrix
(
    GrB_Matrix *Thandle,            // matrix T to build
    const int64_t tnz0,             // final nnz(T)
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **kwork_handle,         // for (i,k) or (j,i,k) tuples
    const GB_void *S,               // array of values of tuples
    const int64_t len,              // number of tuples and size of kwork
    const int64_t ijlen,            // size of iwork array
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S array
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Thandle != NULL) ;
    GrB_Matrix T = *Thandle ;
    ASSERT (T != NULL) ;
    ASSERT (GB_IMPLIES (len > 0, S != NULL)) ;
    ASSERT (iwork_handle != NULL) ;
    ASSERT (kwork_handle != NULL) ;
    ASSERT (scode <= GB_UDT_code) ;

    // if kwork is NULL, then kwork [t] == t
    const int64_t *restrict kwork = *kwork_handle ;
    int64_t *restrict iwork = *iwork_handle ;
    ASSERT (iwork != NULL) ;

    // With GB_build, there can be 1 to 2 different types.
    //      T->type is identical to the types of x,y,z for z=dup(x,y).
    //      dup is never NULL and all its three types are the same
    //      The type of S (scode) can different but must be compatible
    //          with T->type

    // With GB_wait, there can be 1 to 5 different types:
    //      The pending tuples are in S, of type scode which must be
    //          compatible with dup->ytype and T->type
    //      z = dup (x,y): can be NULL or have 1 to 3 different types
    //      T->type: must be compatible with all above types.
    //      dup may be NULL, in which case it is assumed be the implicit SECOND
    //          operator, with all three types equal to T->type

    GrB_Type ttype = T->type, xtype, ytype, ztype ;
    GxB_binary_function fdup ;
    GB_Opcode opcode ;

    GB_Type_code tcode = ttype->code ;
    bool op_2nd ;

    ASSERT_OK (GB_check (ttype, "ttype for build_factorize", GB0)) ;

    if (dup == NULL)
    { 
        // dup is the implicit SECOND operator:
        // z = SECOND (x,y) where all three types are the same as ttype
        // T(i,j) = (ttype) S(k) will be done for all tuples.
        opcode = GB_SECOND_opcode ;
        ASSERT (GB_op_is_second (dup, ttype)) ;
        xtype = ttype ;
        ytype = ttype ;
        ztype = ttype ;
        fdup = NULL ;
        op_2nd = true ;
    }
    else
    { 
        // T(i,j) = (ttype) S[k] will be done for the first tuple.
        // for subsequent tuples: T(i,j) += S[k], via the dup operator and
        // typecasting:
        //
        //      y = (dup->ytype) S[k]
        //      x = (dup->xtype) T(i,j)
        //      z = (dup->ztype) dup (x,y)
        //      T(i,j) = (ttype) z
        ASSERT_OK (GB_check (dup, "dup for build_factory", GB0)) ;
        opcode = dup->opcode ;
        xtype = dup->xtype ;
        ytype = dup->ytype ;
        ztype = dup->ztype ;
        fdup = dup->function ;
        op_2nd = GB_op_is_second (dup, ttype) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // allocate the result
    //--------------------------------------------------------------------------

    // get the sizes and codes of each type
    GB_Type_code zcode = ztype->code ;
    GB_Type_code xcode = xtype->code ;
    GB_Type_code ycode = ytype->code ;

    ASSERT (GB_code_compatible (tcode, scode)) ;    // T(i,j) = (ttype) S
    ASSERT (GB_code_compatible (ycode, scode)) ;    // y = (type) S
    ASSERT (GB_Type_compatible (xtype, ttype)) ;    // x = (xtype) T(i,j)
    ASSERT (GB_Type_compatible (ttype, ztype)) ;    // T(i,j) = (ttype) z

    size_t tsize = ttype->size ;
    size_t zsize = ztype->size ;
    size_t xsize = xtype->size ;
    size_t ysize = ytype->size ;
    size_t ssize = GB_code_size (scode, tsize) ;

    // T->x is allocated fresh but iwork is transplanted in T->i, when done.
    // This allocation is typically free since jwork has just been freed in the
    // caller, GB_builder.  T->x has size tnz*tsize and jwork is size
    // ijlen*sizeof(int64_t).  tnz <= ijlen always holds, and tsize <=
    // size(int64_t) holds for all built-in types.

    T->nzmax = GB_IMAX (tnz0, 1) ;
    ASSERT (tnz0 == GB_NNZ (T)) ;
    ASSERT (tnz0 <= len && tnz0 <= ijlen) ;
    GB_MALLOC_MEMORY (T->x, T->nzmax, tsize) ;
    T->i = NULL ;
    if (T->x == NULL)
    { 
        // out of memory
        GB_MATRIX_FREE (Thandle) ;
        GB_FREE_MEMORY (*kwork_handle, len, sizeof (int64_t)) ;
        GB_FREE_MEMORY (*iwork_handle, ijlen, sizeof (int64_t)) ;
        return (GB_OUT_OF_MEMORY) ;
    }

    GB_void *restrict Tx = T->x ;

    //--------------------------------------------------------------------------
    // assemble the output
    //--------------------------------------------------------------------------

    int64_t tnz = 0 ;

    // so that tcode can match scode
    GB_Type_code tcode2 = (tcode == GB_UCT_code) ? GB_UDT_code : tcode ;
    GB_Type_code scode2 = (scode == GB_UCT_code) ? GB_UDT_code : scode ;

    // no typecasting if all 5 types are the same
    bool nocasting = (tcode2 == scode2) &&
        (ttype == xtype) && (ttype == ytype) && (ttype == ztype) ;

    if (nocasting)
    {

        //----------------------------------------------------------------------
        // assemble the values, S, into T, no typecasting needed
        //----------------------------------------------------------------------

        // There are 44 common cases of this function for built-in types and
        // 8 associative operators: min, max, plus, times for all types;
        // or, and, xor, eq for boolean.

        // In addition, the FIRST and SECOND operators are hard-coded, for
        // another 22 workers, since SECOND is used by GB_wait and since FIRST
        // is useful for keeping the first tuple seen.  It is controlled by the
        // GB_INCLUDE_SECOND_OPERATOR definition, so they do not appear in
        // GB_reduce_to_* where the FIRST and SECOND operators are not needed.

        // Early exit cannot be exploited, so the terminal value is ignored.

        #define GB_INCLUDE_SECOND_OPERATOR

        bool done = false ;

        // define the worker for the switch factory
        #define GB_ASSOC_WORKER(type,ignore)                                \
        {                                                                   \
            const type *restrict sx = (type *) S ;                          \
            type *restrict tx = (type *) Tx ;                               \
            for (int64_t t = 0 ; t < len ; t++)                             \
            {                                                               \
                /* get the (i,k) or (j,i,k) tuple and check if duplicate */ \
                int64_t i = iwork [t] ;                                     \
                int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;           \
                if (i < 0)                                                  \
                {                                                           \
                    /* duplicate entry: Tx [tnz-1] '+=' S [k] */            \
                    GB_DUP (tx [tnz-1], sx [k]) ;                           \
                }                                                           \
                else                                                        \
                {                                                           \
                    /* new entry; save its index and value */               \
                    ASSERT (tnz <= t) ;                                     \
                    tx [tnz] = sx [k] ;                                     \
                    iwork [tnz] = i ;                                       \
                    tnz++ ;                                                 \
                }                                                           \
            }                                                               \
            done = true ;                                                   \
        }                                                                   \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // If GBCOMPACT is defined, the switch factory is disabled and all work
        // is done by the generic worker.  The compiled code will be more
        // compact, but 3 to 4 times slower.

        #ifndef GBCOMPACT

            // controlled by opcode and typecode
            GB_Type_code typecode = tcode ;
            #include "GB_assoc_template.c"

        #endif

        //----------------------------------------------------------------------
        // generic worker
        //----------------------------------------------------------------------

        if (!done)
        {

            //------------------------------------------------------------------
            // no typecasting, but use the fdup function pointer and memcpy
            //------------------------------------------------------------------

            // The switch case did not handle this case.  Any binary operator
            // can be passed to this function, even non-associative built-in
            // operators.  User-defined types are handled here as well, since
            // no typecasting can be done.  This is slower than using
            // hard-coded types and operators.

            // scalar workspace
            size_t size = tsize ;

            if (op_2nd)
            {
                // dup is the explicit or implict SECOND operator
                for (int64_t t = 0 ; t < len ; t++)
                {
                    // get the (i,k) or (j,i,k) tuple and check if duplicate
                    int64_t i = iwork [t] ;
                    int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
                    if (i < 0)
                    { 
                        // duplicate entry:
                        // Tx [tnz-1] = S [k], explicit or implied SECOND
                        memcpy (Tx +((tnz-1)*size), S +(k*size), size) ;
                    }
                    else
                    { 
                        // new entry; save its index and value
                        // Tx [tnz] = S [k]
                        memcpy (Tx +(tnz*size), S +(k*size), size) ;
                        iwork [tnz] = i ;
                        tnz++ ;
                    }
                }
            }
            else
            {
                for (int64_t t = 0 ; t < len ; t++)
                {
                    // get the (i,k) or (j,i,k) tuple and check if duplicate
                    int64_t i = iwork [t] ;
                    int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
                    if (i < 0)
                    { 
                        // duplicate entry: assemble it with the dup operator
                        // Tx [tnz-1] '+=' S [k]
                        GB_void *restrict txt = Tx +((tnz-1)*size) ;
                        fdup (txt, txt, S +(k*size)) ;  // (z x alias)
                    }
                    else
                    { 
                        // new entry; save its index and value
                        // Tx [tnz] = S [k]
                        memcpy (Tx +(tnz*size), S +(k*size), size) ;
                        iwork [tnz] = i ;
                        tnz++ ;
                    }
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // assemble the values S into T, typecasting as needed
        //----------------------------------------------------------------------

        GB_cast_function cast_S_to_T = GB_cast_factory (tcode, scode) ;
        GB_cast_function cast_S_to_Y = GB_cast_factory (ycode, scode) ;
        GB_cast_function cast_T_to_X = GB_cast_factory (xcode, tcode) ;
        GB_cast_function cast_Z_to_T = GB_cast_factory (tcode, zcode) ;

        // The type of the S array differs from the type of T and dup, but both
        // types are built-in since user-defined types cannot be typecasted.
        ASSERT (scode <= GB_FP64_code) ;
        ASSERT (tcode <= GB_FP64_code) ;
        ASSERT (xcode <= GB_FP64_code) ;
        ASSERT (ycode <= GB_FP64_code) ;
        ASSERT (zcode <= GB_FP64_code) ;

        // scalar workspace
        char twork [tsize] ;
        char xwork [xsize] ;
        char ywork [ysize] ;
        char zwork [zsize] ;

        if (op_2nd)
        {

            // dup operator is the implicit SECOND
            for (int64_t t = 0 ; t < len ; t++)
            {
                // get the (i,k) or (j,i,k) tuple and check if duplicate
                int64_t i = iwork [t] ;
                int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
                if (i < 0)
                { 
                    // duplicate entry: Tx [tnz-1] = S [k]
                    // Tx [tnz-1] = (ttype) zwork
                    cast_S_to_T (Tx +((tnz-1)*tsize), S +(k*ssize), 0) ;
                }
                else
                { 
                    // new entry; save its index and value
                    // Tx [tnz] = (ttype) S [k]
                    cast_S_to_T (Tx +(tnz*tsize), S +(k*ssize), 0) ;
                    iwork [tnz] = i ;
                    tnz++ ;
                }
            }

        }
        else
        {

            ASSERT (dup != NULL && fdup != NULL) ;

            // dup operator is not NULL
            for (int64_t t = 0 ; t < len ; t++)
            {
                // get the (i,k) or (j,i,k) tuple and check if duplicate
                int64_t i = iwork [t] ;
                int64_t k = (kwork == NULL) ? (t) : (kwork [t]) ;
                if (i < 0)
                { 
                    // duplicate entry: Tx [tnz-1] '+=' S [k]
                    // ywork = (ytype) S [k]
                    cast_S_to_Y (ywork, S +(k*ssize), 0) ;
                    // xwork = (xtype) Tx [tnz-1]
                    cast_T_to_X (xwork, Tx +((tnz-1)*tsize), 0) ;
                    // zwork = f (xwork, ywork)
                    fdup (zwork, xwork, ywork) ;
                    // Tx [tnz-1] = (ttype) zwork
                    cast_Z_to_T (Tx +((tnz-1)*tsize), zwork, 0) ;
                }
                else
                { 
                    // new entry; save its index and value
                    // Tx [tnz] = (ttype) S [k]
                    cast_S_to_T (Tx +(tnz*tsize), S +(k*ssize), 0) ;
                    iwork [tnz] = i ;
                    tnz++ ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    // In the current implementation, kwork is no longer needed.  If this
    // method were to be split into user-callable symbolic and numerical
    // phases, kwork is part of the symbolic analysis and should be kept for
    // subsequent builds with the same I and J but different S.

    GB_FREE_MEMORY (*kwork_handle, len, sizeof (int64_t)) ;
    kwork = NULL ;

    //--------------------------------------------------------------------------
    // transplant iwork into T->i
    //--------------------------------------------------------------------------

    // shrink iwork from size len to size T->nzmax == tnz0
    if (T->nzmax < ijlen)
    { 
        // this cannot fail since the size is shrinking.
        bool ok ;
        GB_REALLOC_MEMORY (iwork, T->nzmax, ijlen, sizeof (int64_t), &ok,
            Context) ;
        ASSERT (ok) ;
    }
    T->i = iwork ;
    iwork = NULL ;

    // iwork has been transplanted into T, as T->i, so set the iwork pointer
    // in the caller to NULL.
    (*iwork_handle) = NULL ;

    return (GrB_SUCCESS) ;
}


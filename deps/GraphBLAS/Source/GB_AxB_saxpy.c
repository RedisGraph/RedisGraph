//------------------------------------------------------------------------------
// GB_AxB_saxpy: C<M>=A*B or C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Select a saxpy method: gather/scatter or heap

#include "GB.h"

GrB_Info GB_AxB_saxpy               // C=A*B or C<M>=A*B, gather/scatter or heap
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // optional matrix
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected
    GB_Sauna *Sauna_Handle,         // handle to sparse accumulator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // select the type of saxpy method
    //--------------------------------------------------------------------------

    // GB_AxB_heap and GB_AxB_Gustavson compute the same thing (C=A*B or
    // C<M>=A*B), and both use the saxpy method.  They differ in the workspace
    // they use.  GB_AxB_heap uses a heap of size O(b), while GB_AxB_Gustavson
    // uses a Sauna (gather/scatter workspace) of size O(C->vlen).

    // Let b = max (nnz (B (:,j))), for all j; the maximum number of
    // entries in any column of B.

    // GB_AxB_heap uses workspace of 5 * (b+1) * sizeof (int64_t).

    // find the densest column of B
    int64_t b = 0 ;
    GB_for_each_vector (B)
    { 
        int64_t GBI1_initj (Iter, j, pB_start, pB_end) ;
        int64_t bjnz = pB_end - pB_start ;
        b = GB_IMAX (b, bjnz) ;
    }

    double heap_memory = GBYTES (b+1, 5 * sizeof (int64_t)) ;

    // GB_AxB_Gustavson uses a Sauna of size m * csize where C is m-by-n.

    int64_t m = A->vlen ;       // A is m-by-k
    int64_t k = B->vlen ;       // B is k-by-n
    size_t csize = semiring->add->op->ztype->size ;
    double gs_memory = GBYTES (m, csize + sizeof (int64_t)) ;

    bool use_heap ;
    int64_t bnz = GB_NNZ (B) ;
    int64_t anz = GB_NNZ (A) ;

    if (AxB_method == GxB_DEFAULT)
    {
        if (b <= 2)
        { 
            // use the Heap method if all columns of B have 2 entries or less.
            // This is the case if B is a diagonal scaling matrix, a
            // permutation matrix, or upper/lower bidiagonal.  The heap will
            // have size 2, which is very small and will be very fast.
            use_heap = true ;
        }
        else if (bnz <= 3*k || bnz <= m || anz <= GB_IMIN (k,m))
        { 
            // If B is very sparse, with an average of 3 entries per column,
            // then it is a good candidate for the heap method.  The heap
            // method will use O(b) memory, which is at most O(nnz(B)).  The
            // size of A, B, and C could be dwarfed by the O(m) gather/scatter
            // memory, which makes Gustavson's prohibitively expensive.  If A
            // is extremely sparse (the 2nd condition above) then the heap
            // method is also competitive.  In each of these cases, use the
            // heap method if it requires much less memory.

            // The 40*b memory for the heap also comes at the cost of run time;
            // accessing a heap of size O(b) adds an extra factor of O(log(b))
            // to the run time.  Thus the heap method is penalized by a factor
            // of 4*log2(b).

            // The heap always uses 40*b bytes for the heap.  If csize=8
            // (a 64-bit type), then gather/scatter uses 9*m bytes.  So
            // the following rule becomes :

            // use_heap = (4*log(b)*40b < 9*m) or roughly (16*b*log(b)<m).

            use_heap = (4 * log2 ((double) b) * heap_memory < gs_memory) ;
        }
        else
        { 
            // Otherwise, do not use the heap method; use Gustavson's method
            // instead.  Since nnz(B) > m and Gustavson's method requires
            // O(m) workspace, the size of the workspace will be less than the
            // size of the input matrices.  In this case Gustavson's method
            // tends to be faster than the heap method.
            use_heap = false ;
        }
    }
    else
    { 
        // allow the user to select gather/scatter vs heap
        use_heap = (AxB_method == GxB_AxB_HEAP) ;
    }

    //--------------------------------------------------------------------------
    // C<M>=A*B or C=A*B using the selected method
    //--------------------------------------------------------------------------

    if (use_heap)
    { 
        // use saxpy method with a heap; hypersparse matrices will tend to
        // use this option.
        (*AxB_method_used) = GxB_AxB_HEAP ;
        return (GB_AxB_heap (Chandle, M, A, B, semiring, flipxy, b, Context)) ;
    }
    else
    { 
        // use saxpy method with a gather/scatter workspace.
        (*AxB_method_used) = GxB_AxB_GUSTAVSON ;
        return (GB_AxB_Gustavson (Chandle, M, A, B, semiring, flipxy,
            Sauna_Handle, Context)) ;
    }
}


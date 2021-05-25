//------------------------------------------------------------------------------
// GB_AxB_dot_generic: generic template for all dot-product methods
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This template serves all three dot product methods.  The #including file
// defines GB_DOT2_GENERIC, GB_DOT3_GENERIC, or GB_DOT4_GENERIC.

{

    //--------------------------------------------------------------------------
    // get operators, functions, workspace, contents of A, B, C
    //--------------------------------------------------------------------------

    GxB_binary_function fmult = mult->function ;    // NULL if positional
    GxB_binary_function fadd  = add->op->function ;
    GB_Opcode opcode = mult->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;

    size_t csize = C->type->size ;
    size_t asize = A_is_pattern ? 0 : A->type->size ;
    size_t bsize = B_is_pattern ? 0 : B->type->size ;

    size_t xsize = mult->xtype->size ;
    size_t ysize = mult->ytype->size ;

    // scalar workspace: because of typecasting, the x/y types need not
    // be the same as the size of the A and B types.
    // flipxy false: aki = (xtype) A(k,i) and bkj = (ytype) B(k,j)
    // flipxy true:  aki = (ytype) A(k,i) and bkj = (xtype) B(k,j)
    size_t aki_size = flipxy ? ysize : xsize ;
    size_t bkj_size = flipxy ? xsize : ysize ;

    GB_void *GB_RESTRICT terminal = (GB_void *) add->terminal ;

    GB_cast_function cast_A, cast_B ;
    if (flipxy)
    { 
        // A is typecasted to y, and B is typecasted to x
        cast_A = A_is_pattern ? NULL : 
                 GB_cast_factory (mult->ytype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL : 
                 GB_cast_factory (mult->xtype->code, B->type->code) ;
    }
    else
    { 
        // A is typecasted to x, and B is typecasted to y
        cast_A = A_is_pattern ? NULL :
                 GB_cast_factory (mult->xtype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL :
                 GB_cast_factory (mult->ytype->code, B->type->code) ;
    }

    //--------------------------------------------------------------------------
    // C = A'*B via dot products, function pointers, and typecasting
    //--------------------------------------------------------------------------

    #define GB_ATYPE GB_void
    #define GB_BTYPE GB_void
    #define GB_PHASE_2_OF_2

    // no vectorization
    #define GB_PRAGMA_SIMD_VECTORIZE ;
    #define GB_PRAGMA_SIMD_DOT(cij) ;

    if (op_is_positional)
    { 

        //----------------------------------------------------------------------
        // generic semirings with positional multiply operators
        //----------------------------------------------------------------------

        if (flipxy)
        { 
            // flip a positional multiplicative operator
            opcode = GB_binop_flip (opcode) ;
        }

        // aki = A(i,k), located in Ax [pA], value not used
        #define GB_GETA(aki,Ax,pA) ;

        // bkj = B(k,j), located in Bx [pB], value not used
        #define GB_GETB(bkj,Bx,pB) ;

        // define cij for each task
        #define GB_CIJ_DECLARE(cij) GB_CTYPE cij

        // address of Cx [p]
        #define GB_CX(p) (&Cx [p])

        // cij = Cx [p]
        #define GB_GETC(cij,p) cij = Cx [p]

        // Cx [p] = cij
        #define GB_PUTC(cij,p) Cx [p] = cij

        // break if cij reaches the terminal value
        #define GB_DOT_TERMINAL(cij)                                    \
            if (is_terminal && cij == cij_terminal)                     \
            {                                                           \
                break ;                                                 \
            }

        // C(i,j) += (A')(i,k) * B(k,j)
        #define GB_MULTADD(cij, aki, bkj, i, k, j)                      \
            GB_CTYPE zwork ;                                            \
            GB_MULT (zwork, aki, bkj, i, k, j) ;                        \
            fadd (&cij, &cij, &zwork)

        int64_t offset = GB_positional_offset (opcode) ;

        if (mult->ztype == GrB_INT64)
        {
            #define GB_CTYPE int64_t
            int64_t cij_terminal = 0 ;
            bool is_terminal = (terminal != NULL) ;
            if (is_terminal)
            { 
                memcpy (&cij_terminal, terminal, sizeof (int64_t)) ;
            }
            switch (opcode)
            {
                case GB_FIRSTI_opcode   :   // z = first_i(A'(i,k),y) == i
                case GB_FIRSTI1_opcode  :   // z = first_i1(A'(i,k),y) == i+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aki, bkj, i, k, j) t = i + offset
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                case GB_FIRSTJ_opcode   :   // z = first_j(A'(i,k),y) == k
                case GB_FIRSTJ1_opcode  :   // z = first_j1(A'(i,k),y) == k+1
                case GB_SECONDI_opcode  :   // z = second_i(x,B(k,j)) == k
                case GB_SECONDI1_opcode :   // z = second_i1(x,B(k,j)) == k+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aki, bkj, i, k, j) t = k + offset
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                case GB_SECONDJ_opcode  :   // z = second_j(x,B(k,j)) == j
                case GB_SECONDJ1_opcode :   // z = second_j1(x,B(k,j)) == j+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aki, bkj, i, k, j) t = j + offset
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                default: ;
            }
        }
        else
        {
            #undef  GB_CTYPE
            #define GB_CTYPE int32_t
            int32_t cij_terminal = 0 ;
            bool is_terminal = (terminal != NULL) ;
            if (is_terminal)
            { 
                memcpy (&cij_terminal, terminal, sizeof (int32_t)) ;
            }
            switch (opcode)
            {
                case GB_FIRSTI_opcode   :   // z = first_i(A'(i,k),y) == i
                case GB_FIRSTI1_opcode  :   // z = first_i1(A'(i,k),y) == i+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aki,bkj,i,k,j) t = (int32_t) (i + offset)
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                case GB_FIRSTJ_opcode   :   // z = first_j(A'(i,k),y) == k
                case GB_FIRSTJ1_opcode  :   // z = first_j1(A'(i,k),y) == k+1
                case GB_SECONDI_opcode  :   // z = second_i(x,B(k,j)) == k
                case GB_SECONDI1_opcode :   // z = second_i1(x,B(k,j)) == k+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aki,bkj,i,k,j) t = (int32_t) (k + offset)
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                case GB_SECONDJ_opcode  :   // z = second_j(x,B(k,j)) == j
                case GB_SECONDJ1_opcode :   // z = second_j1(x,B(k,j)) == j+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aki,bkj,i,k,j) t = (int32_t) (j + offset)
                    #if defined ( GB_DOT2_GENERIC )
                    #include "GB_AxB_dot2_meta.c"
                    #elif defined ( GB_DOT3_GENERIC )
                    #include "GB_AxB_dot3_meta.c"
                    #else
                    #include "GB_AxB_dot4_meta.c"
                    #endif
                    break ;
                default: ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // generic semirings with standard multiply operators
        //----------------------------------------------------------------------

        // aki = A(k,i), located in Ax [pA]
        #undef  GB_GETA
        #define GB_GETA(aki,Ax,pA)                                      \
            GB_void aki [GB_VLA(aki_size)] ;                            \
            if (!A_is_pattern) cast_A (aki, Ax +((pA)*asize), asize)

        // bkj = B(k,j), located in Bx [pB]
        #undef  GB_GETB
        #define GB_GETB(bkj,Bx,pB)                                      \
            GB_void bkj [GB_VLA(bkj_size)] ;                            \
            if (!B_is_pattern) cast_B (bkj, Bx +((pB)*bsize), bsize)

        // define cij for each task
        #undef  GB_CIJ_DECLARE
        #define GB_CIJ_DECLARE(cij) GB_void cij [GB_VLA(csize)]

        // address of Cx [p]
        #undef  GB_CX
        #define GB_CX(p) Cx +((p)*csize)

        // cij = Cx [p]
        #undef  GB_GETC
        #define GB_GETC(cij,p) memcpy (cij, GB_CX (p), csize)

        // Cx [p] = cij
        #undef  GB_PUTC
        #define GB_PUTC(cij,p) memcpy (GB_CX (p), cij, csize)

        // break if cij reaches the terminal value
        #undef  GB_DOT_TERMINAL
        #define GB_DOT_TERMINAL(cij)                                    \
            if (terminal != NULL && memcmp (cij, terminal, csize) == 0) \
            {                                                           \
                break ;                                                 \
            }

        // C(i,j) += (A')(i,k) * B(k,j)
        #undef  GB_MULTADD
        #define GB_MULTADD(cij, aki, bkj, i, k, j)                      \
            GB_void zwork [GB_VLA(csize)] ;                             \
            GB_MULT (zwork, aki, bkj, i, k, j) ;                        \
            fadd (cij, cij, zwork)

        #undef  GB_CTYPE
        #define GB_CTYPE GB_void

        if (opcode == GB_FIRST_opcode || opcode == GB_SECOND_opcode)
        {
            // fmult is not used and can be NULL (for user-defined types)
            if (flipxy)
            { 
                // flip first and second
                opcode = GB_binop_flip (opcode) ;
            }
            if (opcode == GB_FIRST_opcode)
            { 
                // t = A(i,k)
                ASSERT (B_is_pattern) ;
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, aik, csize)
                #if defined ( GB_DOT2_GENERIC )
                #include "GB_AxB_dot2_meta.c"
                #elif defined ( GB_DOT3_GENERIC )
                #include "GB_AxB_dot3_meta.c"
                #else
                #include "GB_AxB_dot4_meta.c"
                #endif
            }
            else // opcode == GB_SECOND_opcode
            { 
                // t = B(i,k)
                ASSERT (A_is_pattern) ;
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, bkj, csize)
                #if defined ( GB_DOT2_GENERIC )
                #include "GB_AxB_dot2_meta.c"
                #elif defined ( GB_DOT3_GENERIC )
                #include "GB_AxB_dot3_meta.c"
                #else
                #include "GB_AxB_dot4_meta.c"
                #endif
            }
        }
        else
        {
            if (flipxy)
            { 
                // t = B(k,j) * (A')(i,k)
                #undef  GB_MULT
                #define GB_MULT(t, aki, bkj, i, k, j) fmult (t, bkj, aki)
                #if defined ( GB_DOT2_GENERIC )
                #include "GB_AxB_dot2_meta.c"
                #elif defined ( GB_DOT3_GENERIC )
                #include "GB_AxB_dot3_meta.c"
                #else
                #include "GB_AxB_dot4_meta.c"
                #endif
            }
            else
            { 
                // t = (A')(i,k) * B(k,j)
                #undef  GB_MULT
                #define GB_MULT(t, aki, bkj, i, k, j) fmult (t, aki, bkj)
                #if defined ( GB_DOT2_GENERIC )
                #include "GB_AxB_dot2_meta.c"
                #elif defined ( GB_DOT3_GENERIC )
                #include "GB_AxB_dot3_meta.c"
                #else
                #include "GB_AxB_dot4_meta.c"
                #endif
            }
        }
    }
}


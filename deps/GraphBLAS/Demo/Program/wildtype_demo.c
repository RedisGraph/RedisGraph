//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/wildtype_demo: an arbitrary user-defined type
//------------------------------------------------------------------------------

// Each "scalar" entry of this type consists of a 4x4 matrix and a string of
// length 64.

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// the wildtype
//------------------------------------------------------------------------------

typedef struct
{
    float stuff [4][4] ;
    char whatstuff [64] ;
}
wildtype ;                      // C version of wildtype

GrB_Type WildType ;             // GraphBLAS version of wildtype

//------------------------------------------------------------------------------
// wildtype_print: print a "scalar" value of wildtype
//------------------------------------------------------------------------------

void wildtype_print (const wildtype *x, const char *name)
{
    printf ("\na wildtype scalar: %s [%s]\n", name, x->whatstuff) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            printf ("%10.1f ", x->stuff [i][j]) ;
        }
        printf ("\n") ;
    }
}

//------------------------------------------------------------------------------
// wildtype_print_matrix: print a matrix of wildtype scalars
//------------------------------------------------------------------------------

void wildtype_print_matrix (GrB_Matrix A, char *name)
{
    GrB_Type type ;
    GxB_Matrix_type (&type, A) ;
    if (type != WildType)
    {
        printf ("\nThe matrix %s is not wild enough to print.\n", name) ;
        return ;
    }
    GrB_Index nvals, nrows, ncols ;
    GrB_Matrix_nvals (&nvals, A) ;
    GrB_Matrix_nrows (&nrows, A) ;
    GrB_Matrix_ncols (&ncols, A) ;
    printf ("\n============= printing the WildType matrix: %s (%d-by-%d"
        " with %d entries)\n", name, (int) nrows, (int) ncols, (int) nvals) ;
    for (int i = 0 ; i < nrows ; i++)
    {
        for (int j = 0 ; j < ncols ; j++)
        {
            wildtype scalar ;
            GrB_Info info = GrB_Matrix_extractElement_UDT (&scalar, A, i, j) ;
            if (info == GrB_SUCCESS)
            {
                printf ("\n----------- %s(%d,%d):\n", name, i, j) ;
                wildtype_print (&scalar, "") ;
            }
        }
    }
    printf ("\n============= that was the WildType matrix %s\n", name) ;
}

//------------------------------------------------------------------------------
// add two wildtype "scalars"
//------------------------------------------------------------------------------

void wildtype_add (wildtype *z, const wildtype *x, const wildtype *y)
{
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            z->stuff [i][j] = x->stuff [i][j] + y->stuff [i][j] ;
        }
    }
    sprintf (z->whatstuff, "this was added") ;
    printf ("[%s] = [%s] + [%s]\n", z->whatstuff, x->whatstuff, y->whatstuff) ;
}

//------------------------------------------------------------------------------
// multiply two wildtypes "scalars"
//------------------------------------------------------------------------------

void wildtype_mult (wildtype *z, const wildtype *x, const wildtype *y)
{
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            z->stuff [i][j] = 0 ;
            for (int k = 0 ; k < 4 ; k++)
            {
                z->stuff [i][j] += (x->stuff [i][k] * y->stuff [k][j]) ;
            }
        }
    }
    sprintf (z->whatstuff, "this was multiplied") ;
    printf ("[%s] = [%s] * [%s]\n", z->whatstuff, x->whatstuff, y->whatstuff) ;
}

//------------------------------------------------------------------------------
// wildtype main program
//------------------------------------------------------------------------------

int main (void)
{

    // start GraphBLAS
    GrB_init (GrB_NONBLOCKING) ;
    fprintf (stderr, "wildtype_demo:\n") ;

    // create the WildType
    GrB_Type_new (&WildType, wildtype) ;

    // get its properties
    size_t s ;
    GxB_Type_size (&s, WildType) ;
    printf ("WildType size: %d\n", (int) s) ;

    // create a 10-by-10 WildType matrix, each entry is a 'scalar' WildType
    GrB_Matrix A ;
    GrB_Matrix_new (&A, WildType, 10, 10) ;

    wildtype scalar1, scalar2 ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            scalar1.stuff [i][j] = 100*i + j ;
        }
    }
    sprintf (scalar1.whatstuff, "this is from scalar1") ;
    wildtype_print (&scalar1, "scalar1") ;

    // A(2,7) = scalar1
    GrB_Matrix_setElement_UDT (A, &scalar1, 2, 7) ;

    // C = A'
    GrB_Matrix C ;
    GrB_Matrix_new (&C, WildType, 10, 10) ;
    GrB_transpose (C, NULL, NULL, A, NULL) ;

    // scalar2 = C(7,2)
    GrB_Info info = GrB_Matrix_extractElement_UDT (&scalar2, C, 7, 2) ;
    if (info == GrB_SUCCESS)
    {
        wildtype_print (&scalar2, "got scalar2 = C(7,2)") ;
    }
    sprintf (scalar2.whatstuff, "here is scalar2") ;

    // create the WildAdd operator
    GrB_BinaryOp WildAdd ;
    GrB_BinaryOp_new (&WildAdd, wildtype_add, WildType, WildType, WildType) ;

    // create the WildMult operator
    GrB_BinaryOp WildMult ;
    GrB_BinaryOp_new (&WildMult, wildtype_mult, WildType, WildType, WildType) ;

    // create a matrix B with B (7,2) = scalar2
    GrB_Matrix B ;
    GrB_Matrix_new (&B, WildType, 10, 10) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            scalar2.stuff [i][j] = (float) (j - i) + 0.5 ;
        }
    }
    wildtype_print (&scalar2, "scalar2") ;
    GrB_Matrix_setElement_UDT (B, &scalar2, 7, 2) ;

    // create the WildAdder monoid 
    GrB_Monoid WildAdder ;
    wildtype scalar_identity ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            scalar_identity.stuff [i][j] = 0 ;
        }
    }
    sprintf (scalar_identity.whatstuff, "identity") ;
    wildtype_print (&scalar_identity, "scalar_identity for the monoid") ;
    GrB_Monoid_UDT_new (&WildAdder, WildAdd, &scalar_identity) ;

    // create the InTheWild semiring
    GrB_Semiring InTheWild ;
    GrB_Semiring_new (&InTheWild, WildAdder, WildMult) ;

    printf ("\nmultiplication C=A*B InTheWild semiring:\n") ;

    wildtype_print_matrix (A, "input A") ;
    wildtype_print_matrix (B, "input B") ;

    // C = A*B
    GrB_Matrix_clear (C) ;
    GrB_mxm (C, NULL, NULL, InTheWild, A, B, NULL) ;

    wildtype_print_matrix (C, "output C") ;

    // do something invalid
    GrB_Matrix D ;
    GrB_Matrix_new (&D, GrB_FP32, 10, 10) ;
    wildtype_print_matrix (D, "D") ;
    info = GrB_eWiseAdd (C, NULL, NULL, WildAdd, A, D, NULL) ;
    if (info != GrB_SUCCESS)
    {
        printf ("\nThis supposed to fail, as a demo of GrB_error:\n%s\n",
            GrB_error ( )) ;
    }

    // free everyting
    GrB_free (&C) ;
    GrB_free (&A) ;
    GrB_free (&B) ;
    GrB_free (&D) ;
    GrB_free (&InTheWild) ;
    GrB_free (&WildAdder) ;
    GrB_free (&WildAdd) ;
    GrB_free (&WildMult) ;
    GrB_free (&WildType) ;

    GrB_finalize ( ) ;
}


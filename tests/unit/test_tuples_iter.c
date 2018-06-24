#include "assert.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/arithmetic/tuples_iter.h"

void _test_random_vector_iterator() {
    //--------------------------------------------------------------------------
    // Build a random vector
    //--------------------------------------------------------------------------

    GrB_Vector A;
    GrB_Index nvals = 0;
    GrB_Index nrows = 1024;
    GrB_Index *I = malloc(sizeof(GrB_Index) * nrows);
    bool *X = malloc(sizeof(bool) * nrows);

    GrB_Vector_new(&A, GrB_BOOL, nrows);
    
    double mid_point = RAND_MAX/2;
    for(int i = 0; i < nrows; i++) {
        if(rand() > mid_point) {
            I[nvals] = i;
            X[nvals] = true;
            nvals++;
        }
    }

    GrB_Vector_build_BOOL(A, I, X, nvals, GrB_FIRST_BOOL);

    GrB_Index I_expected[nvals];
    GrB_Vector_extractTuples_BOOL(I_expected, NULL, &nvals, A);

    //--------------------------------------------------------------------------
    // Get an iterator over all nonzero elements.
    //--------------------------------------------------------------------------

    TuplesIter *iter = TuplesIter_new((GrB_Matrix)A);
    GrB_Index row;
    
    //--------------------------------------------------------------------------
    // Verify iterator returned values.
    //--------------------------------------------------------------------------
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_OK);
        assert(row == I_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Clean up.
    //--------------------------------------------------------------------------
    TuplesIter_free(iter);
    GrB_Vector_free(&A);
    printf("test_random_vector_iterator - PASS!\n");
}

void _test_vector_iterator() {
    //--------------------------------------------------------------------------
    // Build a vector
    //--------------------------------------------------------------------------

    GrB_Vector A;
    GrB_Vector_new(&A, GrB_BOOL, 4);
    
    GrB_Index nvals = 2;
    GrB_Index I[2] = {1, 3};
    bool X[2] = {true, true};
    GrB_Index I_expected[nvals];

    GrB_Vector_build_BOOL(A, I, X, nvals, GrB_FIRST_BOOL);
    GrB_Vector_extractTuples_BOOL(I_expected, NULL, &nvals, A);

    //--------------------------------------------------------------------------
    // Get an iterator over all vector nonzero elements.
    //--------------------------------------------------------------------------

    TuplesIter *iter = TuplesIter_new((GrB_Matrix)A);
    GrB_Index row;
    
    //--------------------------------------------------------------------------
    // Verify iterator returned values.
    //--------------------------------------------------------------------------
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_OK);
        assert(row == I_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Reset iterator an re-verify.
    //--------------------------------------------------------------------------

    TuplesIter_reset(iter);
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_OK);
        assert(row == I_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, NULL) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Clean up.
    //--------------------------------------------------------------------------
    TuplesIter_free(iter);
    GrB_Vector_free(&A);
    printf("test_vector_iterator - PASS!\n");
}

void _test_random_matrix_iterator() {
    //--------------------------------------------------------------------------
    // Build a random matrix
    //--------------------------------------------------------------------------

    GrB_Matrix A;
    GrB_Index nvals = 0;
    GrB_Index nrows = 1024;
    GrB_Index ncols = 1024;
    GrB_Index *I = malloc(sizeof(GrB_Index) * ncols * nrows);
    GrB_Index *J = malloc(sizeof(GrB_Index) * ncols * nrows);
    bool *X = malloc(sizeof(bool) * ncols * nrows);

    GrB_Matrix_new(&A, GrB_BOOL, nrows, ncols);
    
    double mid_point = RAND_MAX/2;
    for(int i = 0; i < nrows; i++) {
        for(int j = 0; j < ncols; j++) {
            if(rand() > mid_point) {
                I[nvals] = i;
                J[nvals] = j;
                X[nvals] = true;
                nvals++;
            }
        }
    }
    GrB_Matrix_build_BOOL(A, I, J, X, nvals, GrB_FIRST_BOOL);

    GrB_Index *I_expected = malloc(sizeof(GrB_Index) * nvals);
    GrB_Index *J_expected = malloc(sizeof(GrB_Index) * nvals);
    GrB_Matrix_extractTuples_BOOL(I_expected, J_expected, NULL, &nvals, A);

    //--------------------------------------------------------------------------
    // Get an iterator over all matrix nonzero elements.
    //--------------------------------------------------------------------------

    TuplesIter *iter = TuplesIter_new(A);
    GrB_Index row;
    GrB_Index col;
    
    //--------------------------------------------------------------------------
    // Verify iterator returned values.
    //--------------------------------------------------------------------------
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, &col) == TuplesIter_OK);
        assert(row == I_expected[i]);
        assert(col == J_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, &col) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Clean up.
    //--------------------------------------------------------------------------
    free(I_expected);
    free(J_expected);
    TuplesIter_free(iter);
    GrB_Matrix_free(&A);
    printf("test_random_matrix_iterator - PASS!\n");
}

void _test_matrix_iterator() {
    //--------------------------------------------------------------------------
    // Build a 4X4 matrix
    //--------------------------------------------------------------------------

    GrB_Matrix A;
    GrB_Matrix_new(&A, GrB_BOOL, 4, 4);
    
    GrB_Index nvals = 4;
    GrB_Index I[4] = {0,1,2,3};
    GrB_Index J[4] = {0,1,2,3};
    bool X[4] = {true, true, true, true};
    GrB_Index I_expected[nvals];
    GrB_Index J_expected[nvals];

    GrB_Matrix_build_BOOL(A, I, J, X, nvals, GrB_FIRST_BOOL);
    GrB_Matrix_extractTuples_BOOL(I_expected, J_expected, NULL, &nvals, A);

    //--------------------------------------------------------------------------
    // Get an iterator over all matrix nonzero elements.
    //--------------------------------------------------------------------------

    TuplesIter *iter = TuplesIter_new(A);
    GrB_Index row;
    GrB_Index col;
    
    //--------------------------------------------------------------------------
    // Verify iterator returned values.
    //--------------------------------------------------------------------------
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, &col) == TuplesIter_OK);
        assert(row == I_expected[i]);
        assert(col == J_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, &col) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Reset iterator an re-verify.
    //--------------------------------------------------------------------------

    TuplesIter_reset(iter);
    for(int i = 0; i < nvals; i++) {
        assert(TuplesIter_next(iter, &row, &col) == TuplesIter_OK);
        assert(row == I_expected[i]);
        assert(col == J_expected[i]);
    }
    assert(TuplesIter_next(iter, &row, &col) == TuplesIter_DEPLETED);

    //--------------------------------------------------------------------------
    // Clean up.
    //--------------------------------------------------------------------------
    TuplesIter_free(iter);
    GrB_Matrix_free(&A);
    printf("test_matrix_iterator - PASS!\n");
}

int main(int argc, char **argv) {
    GrB_init(GrB_NONBLOCKING);
    _test_matrix_iterator();
    _test_random_matrix_iterator();
    _test_vector_iterator();
    _test_random_vector_iterator();
    GrB_finalize();
    return 0;
}
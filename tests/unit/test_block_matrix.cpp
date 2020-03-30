#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include "../../src/util/rmalloc.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/arithmetic/block_matrix/Include/block_matrix.h"
#ifdef __cplusplus
}
#endif

class BlockMatrixTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
        // Initialization, random seed.
        srand(time(NULL));

        // Initialize GraphBLAS
        GrB_init(GrB_NONBLOCKING);

        // Use the malloc family for allocations
		Alloc_Reset();
	}

    BlockMatrix _random_matrix(GrB_Index nrows, GrB_Index ncols, int nblocks, double sparsity) {
        GrB_Info info;
        BlockMatrix A;
        info = BlockMatrix_new(&A, GrB_BOOL, nrows, ncols, nblocks);
        assert(info == GrB_SUCCESS);
        
        // Populate matrix.    
        for(int row = 0; row < nrows; row++) {
            for(int col = 0; col < ncols; col++) {
                if(((double)rand())/RAND_MAX < sparsity) {
                    info = BlockMatrix_setElement_BOOL(A, true, row, col);
                    assert(info == GrB_SUCCESS);
                }
            }
        }
        
        return A;
    }

    bool _compare_matrices(GrB_Matrix a, GrB_Matrix b) {
        GrB_Index acols, arows, avals;
		GrB_Index bcols, brows, bvals;

		GrB_Matrix_ncols(&acols, a);
		GrB_Matrix_nrows(&arows, a);
		GrB_Matrix_nvals(&avals, a);
		GrB_Matrix_ncols(&bcols, b);
		GrB_Matrix_nrows(&brows, b);
		GrB_Matrix_nvals(&bvals, b);

		if(acols != bcols || arows != brows || avals != bvals) {
			printf("acols: %llu bcols: %llu ", acols, bcols);
			printf("arows: %llu brows: %llu ", arows, brows);
			printf("avals: %llu bvals: %llu", avals, bvals);
            printf("\n");
			return false;
		}

        GrB_Index *aI;     // array for returning row indices of tuples
		GrB_Index *aJ;     // array for returning col indices of tuples
		bool *aX;          // array for returning values of tuples
		GrB_Index *bI;     // array for returning row indices of tuples
		GrB_Index *bJ;     // array for returning col indices of tuples
		bool *bX;          // array for returning values of tuples

        aI = (GrB_Index*)malloc(sizeof(GrB_Index) * avals);
        aJ = (GrB_Index*)malloc(sizeof(GrB_Index) * avals);
        aX = (bool*)malloc(sizeof(bool) * avals);

        bI = (GrB_Index*)malloc(sizeof(GrB_Index) * bvals);
        bJ = (GrB_Index*)malloc(sizeof(GrB_Index) * bvals);
        bX = (bool*)malloc(sizeof(bool) * bvals);

		GrB_Matrix_extractTuples_BOOL(aI, aJ, aX, &avals, a);
		GrB_Matrix_extractTuples_BOOL(bI, bJ, bX, &bvals, b);

		for(int i = 0; i < avals; i++) {
			if(aI[i] != bI[i] || aJ[i] != bJ[i] || aX[i] != bX[i]) {
				printf("Matrix A \n");
                GxB_Matrix_fprint(a, "a", GxB_COMPLETE, stdout);
                printf("\n\n");
				printf("Matrix B \n");
				GxB_Matrix_fprint(b, "b", GxB_COMPLETE, stdout);
				printf("\n\n");
				return false;
			}
		}

        // Clean up.
        free(aI);
        free(aJ);
        free(aX);
        free(bI);
        free(bJ);
        free(bX);

		return true;
	}
};

TEST_F(BlockMatrixTest, TestBlockMatrix_new) {
    BlockMatrix B;
    GrB_Index rows = 10;
    GrB_Index cols = 10;
    GrB_Info info = BlockMatrix_new(&B, GrB_BOOL, rows, cols, 4);
    ASSERT_EQ(info, GrB_SUCCESS);

    GrB_Type type;
    GrB_Index nvals;
    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Index nblocks;
    GrB_Index nblocks_per_row;
    GrB_Index nblocks_per_col;

    info = BlockMatrix_type(&type, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(type, GrB_BOOL);
    
    info = BlockMatrix_BlocksPerRow(&nblocks_per_row, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nblocks_per_row, 2);

    info = BlockMatrix_BlocksPerColumn(&nblocks_per_col, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nblocks_per_col, 2);
    
    info = BlockMatrix_nblocks(&nblocks, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nblocks, 0);
	
    info = BlockMatrix_nrows(&nrows, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nrows, rows);

    info = BlockMatrix_ncols(&ncols, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(ncols, cols);
    
    info = BlockMatrix_nvals(&nvals, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nvals, 0);

	info = BlockMatrix_free(&B);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_setElement) {
    BlockMatrix B;
    int nblocks = 4;
    GrB_Index rows = 10;
    GrB_Index cols = 10;
    GrB_Info info = BlockMatrix_new(&B, GrB_BOOL, rows, cols, nblocks);
    ASSERT_EQ(info, GrB_SUCCESS);

    // Populate matrix.
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            info = BlockMatrix_setElement_BOOL(B, true, row, col);
            ASSERT_EQ(info, GrB_SUCCESS);
        }
    }

    // Out of range set.
    info = BlockMatrix_setElement_BOOL(B, true, rows+1, 0);
    ASSERT_EQ(info, GrB_INVALID_INDEX);
    info = BlockMatrix_setElement_BOOL(B, true, 0, cols+1);
    ASSERT_EQ(info, GrB_INVALID_INDEX);

    // Expecting nvals to report nrow * ncols element.
    GrB_Index nvals;
    info = BlockMatrix_nvals(&nvals, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nvals, rows * cols);    

    // All blocks should be set.
    GrB_Index bnblocks;
    BlockMatrix_nblocks(&bnblocks, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(bnblocks, nblocks);

    // Read matrix content.
    bool x;
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {      
            info = BlockMatrix_extractElement_BOOL(&x, B, row, col);
            ASSERT_EQ(info, GrB_SUCCESS);
            ASSERT_TRUE(x);
        }
    }

    // Out of range get.
    info = BlockMatrix_extractElement_BOOL(&x, B, rows+1, 0);
    ASSERT_EQ(info, GrB_INVALID_INDEX);
    info = BlockMatrix_extractElement_BOOL(&x, B, 0, cols+1);
    ASSERT_EQ(info, GrB_INVALID_INDEX);

    // Clean up.
	info = BlockMatrix_free(&B);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_transpose) {
    GrB_Info info;
    BlockMatrix B;
    BlockMatrix tB;
    GrB_Index rows = 4;
    GrB_Index cols = 10;
    GrB_Index nvals = 0;

    // TODO: Use BlockMatrix_extractTuples when implemented.
    GrB_Index I[rows*cols];
    GrB_Index J[rows*cols];

    info = BlockMatrix_new(&B, GrB_BOOL, rows, cols, 4);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = BlockMatrix_new(&tB, GrB_BOOL, cols, rows, 4);
    ASSERT_EQ(info, GrB_SUCCESS);

    // Populate matrix.    
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            if(((double)rand())/RAND_MAX >= 0.5) {
                info = BlockMatrix_setElement_BOOL(B, true, row, col);
                ASSERT_EQ(info, GrB_SUCCESS);
                I[nvals] = row;
                J[nvals] = col;
                nvals++;
            }
        }
    }

    info = BlockMatrix_transpose(tB, GrB_NULL, GrB_NULL, B, GrB_NULL);
    ASSERT_EQ(info, GrB_SUCCESS);

    // Make number of entries in B and tB is the same.
    GrB_Index tnvals;
    info = BlockMatrix_nvals(&tnvals, tB);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(tnvals, nvals);

    // Validate B[i,j] = tB[j,i]
    for(int i = 0; i < nvals; i++) {
        bool x = false;
        bool tx = false;
        GrB_Index row = I[i];
        GrB_Index col = J[i];

        info = BlockMatrix_extractElement_BOOL(&x, B, row, col);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = BlockMatrix_extractElement_BOOL(&tx, tB, col, row);
        ASSERT_EQ(info, GrB_SUCCESS);
        ASSERT_EQ(x, tx);
    }

    // Clean up.
	info = BlockMatrix_free(&B);
    ASSERT_EQ(info, GrB_SUCCESS);
	info = BlockMatrix_free(&tB);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_flatten) {
    GrB_Info info;
    GrB_Matrix A;
    BlockMatrix B;
    GrB_Index rows = 100;
    GrB_Index cols = 100;

    info = GrB_Matrix_new(&A, GrB_BOOL, rows, cols);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = BlockMatrix_new(&B, GrB_BOOL, rows, cols, 4);
    ASSERT_EQ(info, GrB_SUCCESS);

    // Populate matrix.    
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            if(((double)rand())/RAND_MAX >= 0.5) {
                info = GrB_Matrix_setElement_BOOL(A, true, row, col);
                ASSERT_EQ(info, GrB_SUCCESS);
                info = BlockMatrix_setElement_BOOL(B, true, row, col);
                ASSERT_EQ(info, GrB_SUCCESS);
            }
        }
    }

    GrB_Matrix flat;
    info = BlockMatrix_flatten(&flat, B);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_TRUE(_compare_matrices(A, flat));

    // clean up.
    info = GrB_Matrix_free(&A);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = GrB_Matrix_free(&flat);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = BlockMatrix_free(&B);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_dup) {
    GrB_Info info;
    BlockMatrix A;
    BlockMatrix dup;
    int nblocks = 4;
    GrB_Index nrows = 100;
    GrB_Index ncols = 100;
    A = _random_matrix(nrows, ncols, nblocks, 0.2);
    ASSERT_FALSE(A == NULL);

    // Create a duplicate
    info = BlockMatrix_dup(&dup, A);
    ASSERT_EQ(info, GrB_SUCCESS);

    GrB_Matrix flat;
    GrB_Matrix dup_flat;
    info = BlockMatrix_flatten(&flat, A);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = BlockMatrix_flatten(&dup_flat, dup);
    ASSERT_EQ(info, GrB_SUCCESS);

    ASSERT_TRUE(_compare_matrices(flat, dup_flat));

    // clean up
    info = BlockMatrix_free(&A);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = BlockMatrix_free(&dup);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = GrB_Matrix_free(&flat);
    ASSERT_EQ(info, GrB_SUCCESS);
    info = GrB_Matrix_free(&dup_flat);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_clear) {
    GrB_Info info;
    int nblocks = 4;
    GrB_Index nrows = 100;
    GrB_Index ncols = 100;
    
    BlockMatrix A = _random_matrix(nrows, ncols, nblocks, 0.5);
    ASSERT_FALSE(A == NULL);

    // Make sure matrix isn't empty.
    GrB_Index nvals;
    info = BlockMatrix_nvals(&nvals, A);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_GT(nvals, 0);

    info = BlockMatrix_clear(A);
    ASSERT_EQ(info, GrB_SUCCESS);

    // Make sure matrix is empty.
    info = BlockMatrix_nvals(&nvals, A);
    ASSERT_EQ(info, GrB_SUCCESS);
    ASSERT_EQ(nvals, 0);

    // clean up
    info = BlockMatrix_free(&A);
    ASSERT_EQ(info, GrB_SUCCESS);
}

TEST_F(BlockMatrixTest, TestBlockMatrix_mxm) {
    GrB_Info info;
    GrB_Index nrows[2] = {100, 10000};
    GrB_Index ncols[2] = {100, 10000};
    int nblocks[2] = {4, 25};

    // Multiply 2 random matrices
    GrB_Type type;
    BlockMatrix A;
    BlockMatrix B;
    BlockMatrix C;
    GrB_Matrix aflat;
    GrB_Matrix bflat;
    GrB_Matrix cflat;

    for(int i = 0; i < 2; i++) {
        A = _random_matrix(nrows[i], ncols[i], nblocks[i], 0.01);
        B = _random_matrix(nrows[i], ncols[i], nblocks[i], 0.01);
        info = BlockMatrix_type(&type, A);
        ASSERT_EQ(info, GrB_SUCCESS);
        BlockMatrix_new(&C, type, nrows[i], ncols[i], nblocks[i]);
        ASSERT_EQ(info, GrB_SUCCESS);
        
        // Perform flat multiplication.
        info = BlockMatrix_flatten(&aflat, A);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = BlockMatrix_flatten(&bflat, B);
        ASSERT_EQ(info, GrB_SUCCESS);    
        // aflat = aflat * bflat.
        info = GrB_mxm(aflat, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, aflat, bflat, GrB_NULL);

        // Perform block multiplication.
        info = BlockMatrix_mxm(C, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A, B, GrB_NULL);        
        ASSERT_EQ(info, GrB_SUCCESS);

        // Flatten C.
        info = BlockMatrix_flatten(&cflat, C);
        ASSERT_EQ(info, GrB_SUCCESS);

        // Verify.
        ASSERT_TRUE(_compare_matrices(cflat, aflat));

        // clean up
        info = BlockMatrix_free(&A);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = BlockMatrix_free(&B);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = BlockMatrix_free(&C);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = GrB_Matrix_free(&aflat);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = GrB_Matrix_free(&bflat);
        ASSERT_EQ(info, GrB_SUCCESS);
        info = GrB_Matrix_free(&cflat);
        ASSERT_EQ(info, GrB_SUCCESS);
    }
}

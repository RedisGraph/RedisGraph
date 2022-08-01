/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "../../src/util/rmalloc.h"
#include "../../src/logic/algorithms/pagerank.h"

#ifdef __cplusplus
}
#endif

class PagerankTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(PagerankTest, Pagerank) {
	GrB_init(GrB_NONBLOCKING);

	GrB_Matrix A;
	double tol = 1e-4 ;
	int iters, itermax = 100 ;
	LAGraph_PageRank *ranking;

	/* Graph on the cover of the book, 'Graph Algorithms in the language of linear algebra'.
	A = [
	    0 0 0 1 0 0 0
	    1 0 0 0 0 0 0
	    0 0 0 1 0 1 1
	    1 0 0 0 0 0 1
	    0 1 0 0 0 0 1
	    0 0 1 0 1 0 0
	    0 1 0 0 0 0 0 ] ;
	*/

	GrB_Matrix_new(&A, GrB_BOOL, 7, 7);
	GrB_Matrix_setElement_BOOL(A, true, 3, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);
	GrB_Matrix_setElement_BOOL(A, true, 3, 2);
	GrB_Matrix_setElement_BOOL(A, true, 5, 2);
	GrB_Matrix_setElement_BOOL(A, true, 6, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 3);
	GrB_Matrix_setElement_BOOL(A, true, 6, 3);
	GrB_Matrix_setElement_BOOL(A, true, 1, 4);
	GrB_Matrix_setElement_BOOL(A, true, 6, 4);
	GrB_Matrix_setElement_BOOL(A, true, 2, 5);
	GrB_Matrix_setElement_BOOL(A, true, 4, 5);
	GrB_Matrix_setElement_BOOL(A, true, 1, 6);

	Pagerank(&ranking, A, itermax, tol, &iters);

	/*
	Page:5, pagerank:0.392289
	Page:2, pagerank:0.387241
	Page:3, pagerank:0.050505
	Page:4, pagerank:0.049130
	Page:0, pagerank:0.042893
	Page:1, pagerank:0.039658
	Page:6, pagerank:0.038283
	*/
	LAGraph_PageRank expectations[7] = {
		{5, 0.392289},
		{2, 0.387241},
		{3, 0.050505},
		{4, 0.049130},
		{0, 0.042893},
		{1, 0.039658},
		{6, 0.038283},
	};

	for(int i = 0; i < 7; i++) {
		ASSERT_NEAR(ranking[i].page, expectations[i].page, 0.000001);
		ASSERT_NEAR(ranking[i].pagerank, expectations[i].pagerank, 0.000001);
	}

	GrB_finalize();
}

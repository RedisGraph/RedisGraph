/*
* Copyright 2018-2019 Redis Labs OP_Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"
#include "../../src/execution_plan/ops/ops.h"
#ifdef __cplusplus
}
#endif

class ExecutionPlanOpsTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations.
		Alloc_Reset();
	}
};

//------------------------------------------------------------------------------
// Op Argument
//------------------------------------------------------------------------------
TEST_F(ExecutionPlanOpsTest, OpArgument) {
    /* Create a new Argument operation 
     * verify initial state. */
    Argument *op = (Argument*)NewArgumentOp();
    ASSERT_TRUE(op->r == NULL); // Internal record should be NULL.
    
    // Set operation internal record.
    Record r = Record_New(3);
    ArgumentSetRecord(op, Record_Clone(r));
    ASSERT_TRUE(op->r != NULL);

    // Reset operation.
    ASSERT_EQ(ArgumentReset((OpBase*)op), OP_OK);
    ASSERT_TRUE(op->r == NULL); // Reset should set `r` to NULL.

    // Set operation internal record.
    ArgumentSetRecord(op, Record_Clone(r));
    ASSERT_TRUE(op->r != NULL);

    // Ask operation for data, expecting a single record.
    r = ArgumentConsume((OpBase*)op);
    ASSERT_TRUE(r != NULL);

    // Ask operation for data, a second time, expecting NULL.
    r = ArgumentConsume((OpBase*)op);
    ASSERT_TRUE(r == NULL);
    
    ArgumentFree((OpBase*)op);
}

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
#include "../../src/arithmetic/funcs.h"
#include "../../src/execution_plan/ops/ops.h"
#include "../../src/execution_plan/execution_plan.h"
#ifdef __cplusplus
}
#endif

class ExecutionPlanOpsTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations.
		Alloc_Reset();
	    // Register arithmetic functions
		AR_RegisterFuncs();
	}

	static void TearDownTestCase() {
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

//------------------------------------------------------------------------------
// Op Semi Apply
//------------------------------------------------------------------------------
TEST_F(ExecutionPlanOpsTest, OpSemiApply) {
    SemiApply *op = (SemiApply*)NewSemiApplyOp();
    ASSERT_TRUE(op->r == NULL);
    ASSERT_TRUE(op->op_arg == NULL);
    
    OpBase *left_branch;
    OpBase *right_branch;

    AR_ExpNode *empty_array = AR_EXP_NewOpNode("tolist", 0);
    AR_ExpNode *tiny_array = AR_EXP_NewOpNode("tolist", 3);
    tiny_array->op.children[0] = AR_EXP_NewConstOperandNode(SI_LongVal(0));
    tiny_array->op.children[1] = AR_EXP_NewConstOperandNode(SI_LongVal(1));
    tiny_array->op.children[2] = AR_EXP_NewConstOperandNode(SI_LongVal(2));

    left_branch = NewUnwindOp(0, AR_EXP_Clone(tiny_array));
    ExecutionPlan_AddOp((OpBase*)op, left_branch);

    right_branch = NewUnwindOp(0, AR_EXP_Clone(empty_array));
    Argument *arg = (Argument*)NewArgumentOp();
    ExecutionPlan_AddOp(right_branch, (OpBase *)arg);
    ExecutionPlan_AddOp((OpBase*)op, right_branch);

    SemiApplyInit((OpBase*)op);
    Record r = SemiApplyConsume((OpBase *)op);
    ASSERT_TRUE(r == NULL);

    SemiApplyFree((OpBase *)op);
    ArgumentFree((OpBase *)arg);
    UnwindFree((OpBase *)left_branch);
    UnwindFree((OpBase *)right_branch);
    
    //------------------------------------------------------------------------------

    op = (SemiApply*)NewSemiApplyOp();

    tiny_array = AR_EXP_NewOpNode("tolist", 3);
    tiny_array->op.children[0] = AR_EXP_NewConstOperandNode(SI_LongVal(0));
    tiny_array->op.children[1] = AR_EXP_NewConstOperandNode(SI_LongVal(1));
    tiny_array->op.children[2] = AR_EXP_NewConstOperandNode(SI_LongVal(2));

    left_branch = NewUnwindOp(0, AR_EXP_Clone(tiny_array));
    ExecutionPlan_AddOp((OpBase*)op, left_branch);

    right_branch = NewUnwindOp(0, AR_EXP_Clone(tiny_array));
    arg = (Argument*)NewArgumentOp();
    ExecutionPlan_AddOp(right_branch, (OpBase *)arg);
    ExecutionPlan_AddOp((OpBase*)op, right_branch);

    SemiApplyInit((OpBase*)op);
    for(int i = 0; i < 3; i++) {
        r = SemiApplyConsume((OpBase *)op);    
        ASSERT_TRUE(r != NULL);
    }
    r = SemiApplyConsume((OpBase *)op);    
    ASSERT_TRUE(r == NULL);

    SemiApplyFree((OpBase *)op);
    ArgumentFree((OpBase *)arg);
    UnwindFree((OpBase *)left_branch);
    UnwindFree((OpBase *)right_branch);
   
    //------------------------------------------------------------------------------

    AR_EXP_Free(tiny_array);
    AR_EXP_Free(empty_array);
}

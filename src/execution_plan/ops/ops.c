#include "ops.h"

void OpDesc_RegisterOps() {
    OpAggregateRegister();
    OpAllNodeScanRegister();
    OpApplyMultiplexerRegister();
    OpApplyRegister();
    OpArgumentRegister();
    OpCartesianProductRegister();
    OpCondVarLenTraverseRegister();
    OpCondTraverseRegister();
    OpCreateRegister();
    OpDeleteRegister();
    OpDistinctRegister();
    OpEdgeIndexScanRegister();
    OpExpandIntoRegister();
    OpFilterRegister();
    OpJoinRegister();
    OpLimitRegister();
    OpMergeCreateRegister();
    OpMergeRegister();
    OpNodeByIdSeekRegister();
    OpNodeIndexScanRegister();
    OpNodeLabelScanRegister();
    OpOptionalRegister();
    OpProcCallRegister();
    OpProjectRegister();
    OpResultRegister();
    OpSemiApplyRegister();
    OpSkipRegister();
    OpSortRegister();
    OpUnwindRegister();
    OpUpdateRegister();
    OpValueHashJoinRegister();
}
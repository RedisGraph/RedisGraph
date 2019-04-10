/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "seek_by_id.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "../ops/op_index_scan.h"
#include "../ops/op_all_node_scan.h"
#include "../ops/op_node_by_id_seek.h"
#include "../ops/op_node_by_label_scan.h"

static bool _idFilter(FT_FilterNode *f, int *rel, EntityID *id) {
    if(f->t == FT_N_COND) return false;
    if(f->pred.op == NE) return false;
    
    AR_OpNode *op;
    AR_OperandNode *operand;
    AR_ExpNode *lhs = f->pred.lhs;
    AR_ExpNode *rhs = f->pred.rhs;
    *rel = f->pred.op;

    /* Either ID(N) compare const
     * OR
     * const compare ID(N) */
    if(lhs->type == AR_EXP_OPERAND && rhs->type == AR_EXP_OP) {
        op = &rhs->op;
        operand = &lhs->operand;
    } else if(lhs->type == AR_EXP_OP && rhs->type == AR_EXP_OPERAND) {
        op = &lhs->op;
        operand = &rhs->operand;
    } else {
        return false;
    }

    // Make sure ID is compared to a constant.
    if(operand->type != AR_EXP_CONSTANT) return false;
    if(SI_TYPE(operand->constant) != T_INT64) return false;
    *id = SI_GET_NUMERIC(operand->constant);

    // Make sure applied function is ID.
    if(strcasecmp(op->func_name, "id")) return false;

    return true;
}

static void _setupIdRange(int rel, EntityID id, NodeID *minId, NodeID *maxId, bool *includeMin, bool *includeMax) {
    switch(rel) {
        case GT:
            *minId = id;
            break;
        case GE:
            *minId = id;
            *includeMin = true;
            break;
        case LT:
            *maxId = id;
            break;
        case LE:
            *maxId = id;
            *includeMax = true;
            break;
        case EQ:
            *minId = id;
            *maxId = id;
            *includeMin = true;
            *includeMax = true;
            break;
        default:
            assert(false);
            break;
    }
}

void _reduceTap(ExecutionPlan *plan, const AST *ast, OpBase *tap) {
    if(tap->type == OPType_ALL_NODE_SCAN ||
       tap->type == OPType_NODE_BY_LABEL_SCAN ||
       tap->type == OPType_INDEX_SCAN) {
        /* See if there's a filter of the form
         * ID(n) = X
         * where X is a constant. */
        OpBase *parent = tap->parent;
        while(parent && parent->type == OPType_FILTER) {
            Filter *filter = (Filter*)parent;
            FT_FilterNode *f = filter->filterTree;

            int rel;
            EntityID id;
            if(_idFilter(f, &rel, &id)) {
                int nodeRecIdx = -1;
                NodeID minId = ID_RANGE_UNBOUND;
                NodeID maxId = ID_RANGE_UNBOUND;
                bool includeMin = false;
                bool includeMax = false;
                OpBase *opNodeByIdSeek;

                if(tap->type == OPType_ALL_NODE_SCAN)
                    nodeRecIdx = ((AllNodeScan*)tap)->nodeRecIdx;
                else if(tap->type == OPType_NODE_BY_LABEL_SCAN)
                    nodeRecIdx = ((NodeByLabelScan*)tap)->nodeRecIdx;
                else if(tap->type == OPType_INDEX_SCAN)
                    nodeRecIdx = ((IndexScan*)tap)->nodeRecIdx;
                
                _setupIdRange(rel, id, &minId, &maxId, &includeMin, &includeMax);
                opNodeByIdSeek = NewOpNodeByIdSeekOp(ast, nodeRecIdx, minId, maxId,
                    includeMin, includeMax);

                // Managed to reduce!
                ExecutionPlan_ReplaceOp(plan, tap, opNodeByIdSeek);
                ExecutionPlan_RemoveOp(plan, (OpBase*)filter);
                break;
            }

            // Advance.
            parent = parent->parent;
        }
    }
}

void seekByID(ExecutionPlan *plan, const AST *ast) {
    assert(plan);
    
    OpBase **taps = array_new(OpBase*, 1);
    ExecutionPlan_Taps(plan->root, &taps);

    for(int i = 0; i < array_len(taps); i++) {
        _reduceTap(plan, ast, taps[i]);
    }

    array_free(taps);
}

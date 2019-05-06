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

static bool _idFilter(FT_FilterNode *f, int *rel, EntityID *id, bool *reverse) {
    if(f->t == FT_N_COND) return false;
    if(f->pred.op == OP_NEQUAL) return false;
    
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
        *reverse = true;
    } else if(lhs->type == AR_EXP_OP && rhs->type == AR_EXP_OPERAND) {
        op = &lhs->op;
        operand = &rhs->operand;
        *reverse = false;
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

static void _setupIdRange(int rel, EntityID id, bool reverse, NodeID *minId, NodeID *maxId, bool *inclusiveMin, bool *inclusiveMax) {
    switch(rel) {
        case OP_GT:
            *minId = id;
            break;
        case OP_GE:
            *minId = id;
            *inclusiveMin = true;
            break;
        case OP_LT:
            *maxId = id;
            break;
        case OP_LE:
            *maxId = id;
            *inclusiveMax = true;
            break;
        case OP_EQUAL:
            *minId = id;
            *maxId = id;
            *inclusiveMin = true;
            *inclusiveMax = true;
            break;
        default:
            assert(false);
            break;
    }
    
    /* WHERE ID(n) >= 5
     * Reverse
     * WHERE 5 <= ID(n) */
    if(reverse) {
        NodeID tmpNodeId;
        bool tmpInclusive;

        tmpNodeId = *minId;
        *minId = *maxId;
        *maxId = tmpNodeId;

        tmpInclusive = *inclusiveMin;
        *inclusiveMin = *inclusiveMax;
        *inclusiveMax = tmpInclusive;
    }
}

void _reduceTap(ExecutionPlanSegment *plan, OpBase *tap) {
    if(tap->type & OP_SCAN) {
        /* See if there's a filter of the form
         * ID(n) = X
         * where X is a constant. */
        OpBase *parent = tap->parent;
        while(parent && parent->type == OPType_FILTER) {
            Filter *filter = (Filter*)parent;
            FT_FilterNode *f = filter->filterTree;

            int rel;
            EntityID id;
            bool reverse;
            if(_idFilter(f, &rel, &id, &reverse)) {
                int nodeRecIdx = -1;
                NodeID minId = ID_RANGE_UNBOUND;
                NodeID maxId = ID_RANGE_UNBOUND;
                bool inclusiveMin = false;
                bool inclusiveMax = false;
                OpBase *opNodeByIdSeek;

                switch(tap->type) {
                    case OPType_ALL_NODE_SCAN:
                        nodeRecIdx = ((AllNodeScan*)tap)->nodeRecIdx;
                        break;
                    case OPType_NODE_BY_LABEL_SCAN:                
                        nodeRecIdx = ((NodeByLabelScan*)tap)->nodeRecIdx;
                        break;
                    case OPType_INDEX_SCAN:
                        nodeRecIdx = ((IndexScan*)tap)->nodeRecIdx;
                        break;
                    default:
                        assert(false);
                }

                _setupIdRange(rel, id, reverse, &minId, &maxId, &inclusiveMin, &inclusiveMax);
                opNodeByIdSeek = NewOpNodeByIdSeekOp(nodeRecIdx, minId, maxId,
                    inclusiveMin, inclusiveMax);

                // Managed to reduce!
                ExecutionPlanSegment_ReplaceOp(plan, tap, opNodeByIdSeek);
                ExecutionPlanSegment_RemoveOp(plan, (OpBase*)filter);
                break;
            }

            // Advance.
            parent = parent->parent;
        }
    }
}

void seekByID(ExecutionPlanSegment *plan) {
    assert(plan);
    
    OpBase **taps = array_new(OpBase*, 1);
    ExecutionPlanSegment_Taps(plan->root, &taps);

    for(int i = 0; i < array_len(taps); i++) {
        _reduceTap(plan, taps[i]);
    }

    array_free(taps);
}

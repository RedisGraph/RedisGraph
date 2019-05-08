/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

static AR_ExpNode** _getOrderExpressions(OpBase *op) {
    if(op == NULL) return NULL;
    // No need to look further if we haven't encountered a sort operation
    // before a project/aggregate op
    if (op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return NULL;

    if(op->type == OPType_SORT) {
        OpSort *sort = (OpSort*)op;
        return sort->expressions;
    }
    return _getOrderExpressions(op->parent);
}

OpBase* NewProjectOp(AR_ExpNode **exps, uint *modifies) {
    AST *ast = AST_GetFromTLS();
    OpProject *project = malloc(sizeof(OpProject));
    project->ast = ast;
    project->exps = exps;
    project->exp_count = array_len(exps);
    project->order_exps = NULL;
    project->order_exp_count = 0;
    project->singleResponse = false;

    // Set our Op operations
    OpBase_Init(&project->op);
    project->op.name = "Project";
    project->op.type = OPType_PROJECT;
    project->op.consume = ProjectConsume;
    project->op.init = ProjectInit;
    project->op.reset = ProjectReset;
    project->op.free = ProjectFree;

    project->op.modifies = modifies;

    return (OpBase*)project;
}

OpResult ProjectInit(OpBase *opBase) {
    OpProject *op = (OpProject*)opBase;
    AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
    if (order_exps) {
        op->order_exps = order_exps;
        op->order_exp_count = array_len(order_exps);
    }

    return OP_OK;
}

Record ProjectConsume(OpBase *opBase) {
    OpProject *op = (OpProject*)opBase;
    Record r = NULL;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        r = child->consume(child);
        if(!r) return NULL;
    } else {
        // QUERY: RETURN 1+2
        // Return a single record followed by NULL
        // on the second call.
        if(op->singleResponse) return NULL;
        op->singleResponse = true;
        r = Record_New(0);  // Fake empty record.
    }

    Record projection = Record_New(op->exp_count + op->order_exp_count);
    int rec_idx = 0;
    for(unsigned short i = 0; i < op->exp_count; i++) {
        SIValue v = AR_EXP_Evaluate(op->exps[i], r);
        switch(SI_TYPE(v)) {
            case T_NODE:
                Record_AddNode(projection, rec_idx, *((Node*)v.ptrval));
                break;
            case T_EDGE:
                Record_AddEdge(projection, rec_idx, *((Edge*)v.ptrval));
                break;
            default:
                Record_AddScalar(projection, rec_idx, v);
                break;
        }
        rec_idx++;
    }

    // Project Order expressions.
    for(unsigned short i = 0; i < op->order_exp_count; i++) {
        SIValue v = AR_EXP_Evaluate(op->order_exps[i], r);
        switch(SI_TYPE(v)) {
            case T_NODE:
                Record_AddNode(projection, rec_idx, *((Node*)v.ptrval));
                break;
            case T_EDGE:
                Record_AddEdge(projection, rec_idx, *((Edge*)v.ptrval));
                break;
            default:
                Record_AddScalar(projection, rec_idx, v);
                break;
        }
        rec_idx++;
    }

    Record_Free(r);
    return projection;
}

OpResult ProjectReset(OpBase *ctx) {
    return OP_OK;
}

void ProjectFree(OpBase *ctx) {
    OpProject *op = (OpProject*)ctx;

    // for(unsigned short i = 0; i < op->exp_count; i++) AR_EXP_Free(op->exps[i]); // TODO double free
    // array_free(op->exps);
}

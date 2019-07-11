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

 /**
  * @brief Counts the actual aliases inside the aliases array. 
  * The aliases array is an array with size of the original projected values.
  * It holds the mapping between a variable and its alias, or null if such alias does not exists.
  * We need to use this function in order to allow a projected record to hold both its 
  * projected variables values, and their alises values. If a record is allocated only with the
  * length of the size of the expression variables count, it cannot store its alises values,
  * and will cause a memory leak.
  * @param  aliases: aliases mapping array
  * @retval the amount of alised variables.
  */
int _actualAliasesCount(char** aliases){
    int count = 0;
    int aliasesLen = array_len(aliases);
    for (int i = 0; i < aliasesLen; i++){
        if(aliases[i]) count++;
    }
    return count;
}

OpBase* NewProjectOp(const AST *ast, AR_ExpNode **exps, char **aliases) {
    OpProject *project = malloc(sizeof(OpProject));
    project->ast = ast;
    project->exps = exps;
    project->exp_count = array_len(exps);
    project->order_exps = NULL;
    project->order_exp_count = 0;
    project->singleResponse = false;
    project->aliases = aliases;
    project->record_len = project->exp_count;
    project->record_len += _actualAliasesCount(aliases);
    // Set our Op operations
    OpBase_Init(&project->op);
    project->op.name = "Project";
    project->op.type = OPType_PROJECT;
    project->op.consume = ProjectConsume;
    project->op.init = ProjectInit;
    project->op.reset = ProjectReset;
    project->op.free = ProjectFree;

    project->op.modifies = NewVector(char*, 0);
    for(uint i = 0; i < array_len(aliases); i++) {
        if(aliases[i]) Vector_Push(project->op.modifies, aliases[i]);
    }

    return (OpBase*)project;
}

OpResult ProjectInit(OpBase *opBase) {
    OpProject *op = (OpProject*)opBase;
    AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
    if (order_exps) {
        op->order_exps = order_exps;
        op->order_exp_count = array_len(order_exps);
        op->record_len += op->order_exp_count;
    }

    return OP_OK;
}

Record ProjectConsume(OpBase *opBase) {
    OpProject *op = (OpProject*)opBase;
    Record r = NULL;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        r = OpBase_Consume(child);
        if(!r) return NULL;
    } else {
        // QUERY: RETURN 1+2
        // Return a single record followed by NULL
        // on the second call.
        if(op->singleResponse) return NULL;
        op->singleResponse = true;
        r = Record_New(AST_AliasCount(op->ast));  // Fake empty record.
    }

    Record projection = Record_New(op->record_len);
    int rec_idx = 0;
    for(unsigned short i = 0; i < op->exp_count; i++) {
        SIValue v = AR_EXP_Evaluate(op->exps[i], r);
        /* Incase expression is aliased, add it to record
         * as it might be referenced by other expressions:
         * e.g. RETURN n.v AS X ORDER BY X * X
         * WITH 1 as one, one+one as two */
        char *alias = op->aliases[i];

        Record_Add(projection, rec_idx, v);
        if (alias) Record_Add(r, AST_GetAliasID(op->ast, alias), v);

        rec_idx++;
    }

    // Project Order expressions.
    for(unsigned short i = 0; i < op->order_exp_count; i++) {
        SIValue v = AR_EXP_Evaluate(op->order_exps[i], r);
        Record_Add(projection, rec_idx, v);
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

    for(unsigned short i = 0; i < op->exp_count; i++) AR_EXP_Free(op->exps[i]);
    array_free(op->exps);
    array_free(op->aliases);
}

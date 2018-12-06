/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_orderby.h"

#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"

static bool _record_islt(Record *a, Record *b, const OrderBy *op) {    
    AR_ExpNode **fields = op->fields;
    uint fieldsCount = array_len(fields);

    for(int i = 0; i < fieldsCount; i++) {
        const AR_ExpNode *field = fields[i];
        char *fieldStr = NULL;
        AR_EXP_ToString(field, &fieldStr);        
        SIValue aVal = Record_GetScalar(*a, fieldStr);
        SIValue bVal = Record_GetScalar(*b, fieldStr);
        int rel = SIValue_Order(aVal, bVal);
        if(rel == 0) continue;  // Elements are equal; try next ORDER BY element
        rel *= op->direction;   // Flip value for descending order.
        return rel > 0;         // Return true if the current left element is less than the right.
    }
    return false;   // b >= a
}

// Compares two records on a subset of fields
// Return value similar to strcmp
static int _record_compare(Record *a, Record *b, const OrderBy *op) {
    AR_ExpNode **fields = op->fields;
    uint fieldsCount = array_len(fields);

    for(int i = 0; i < fieldsCount; i++) {
        const AR_ExpNode *field = fields[i];
        char *fieldStr = NULL;
        AR_EXP_ToString(field, &fieldStr);
        SIValue aVal = Record_GetScalar(*a, fieldStr);
        SIValue bVal = Record_GetScalar(*b, fieldStr);
        int rel = SIValue_Order(aVal, bVal);
        if(rel) return rel;
    }
    return 0;
}

// Compares two heap record nodes.
static int _heap_elem_compare(const void *A, const void *B, const void *udata) {
    OrderBy* op = (OrderBy*)udata;
    Record *aRec = (Record*)A;
    Record *bRec = (Record*)B;
    return _record_compare(aRec, bRec, op) * op->direction;
}

static void _accumulate(OrderBy *op, Record *r) {
    AR_ExpNode **fields = op->fields;
    uint fieldsCount = array_len(fields);
    char *fieldStr = NULL;

    for(int i = 0; i < fieldsCount; i++) {
        const AR_ExpNode *field = fields[i];
        AR_EXP_ToString(field, &fieldStr);
        if(!Record_ContainsKey(*r, fieldStr)) {
            SIValue v = AR_EXP_Evaluate(field, *r);
            Record_AddScalar(r, fieldStr, v);
        }
    }

    if(!op->limit) {
        /* Not using a heap and there's room for record. */
        op->buffer = array_append(op->buffer, r);
        return;
    }

    if(heap_count(op->heap) < op->limit) {
        heap_offer(&op->heap, r);
    } else {
        // No room in the heap, see if we need to replace
        // a heap stored record with the current record.
        if(_heap_elem_compare(heap_peek(op->heap), r, op) > 0) {
            Record *replaced = heap_poll(op->heap);
            heap_offer(&op->heap, r);
        }
    }
}

static Record* _handoff(OrderBy *op) {
    if(array_len(op->buffer) > 0) return array_pop(op->buffer);
    return NULL;
}

OpBase *NewOrderByOp(const AST_Query *ast) {
    OrderBy *orderBy = malloc(sizeof(OrderBy));
    orderBy->direction = (ast->orderNode->direction == ORDER_DIR_DESC) ? DIR_DESC : DIR_ASC;
    orderBy->limit = 0;
    orderBy->heap = NULL;
    orderBy->buffer = NULL;
    orderBy->fields = array_new(AR_ExpNode*, array_len(ast->orderNode->fields));

    if(ast->limitNode) {
        orderBy->limit = ast->limitNode->limit;
        if(ast->skipNode) {
            orderBy->limit += ast->skipNode->skip;
        }
    }
    
    for(int i = 0; i < array_len(ast->orderNode->fields); i++) {
        AST_ArithmeticExpressionNode *exp = ast->orderNode->fields[i];
        
        // TODO: Handle alias resolution at a previous stage.
        if(exp->type == AST_AR_EXP_OPERAND &&
           exp->operand.type == AST_AR_EXP_VARIADIC && 
           exp->operand.variadic.alias != NULL &&
           exp->operand.variadic.property == NULL) {
            size_t returnElementsCount = Vector_Size(ast->returnNode->returnElements);
            for(int j = 0; j < returnElementsCount; j++) {
                AST_ReturnElementNode *returnElem;
                Vector_Get(ast->returnNode->returnElements, j, &returnElem);
                if(returnElem->alias && strcmp(returnElem->alias, exp->operand.variadic.alias) == 0) {
                    exp = returnElem->exp;
                }
            }
        }

        orderBy->fields = array_append(orderBy->fields, AR_EXP_BuildFromAST(exp));
    }

    if(orderBy->limit) orderBy->heap = heap_new(_heap_elem_compare, orderBy);
    else orderBy->buffer = array_new(Record*, 32);

    // Set our Op operations
    OpBase_Init(&orderBy->op);
    orderBy->op.name = "OrderBy";
    orderBy->op.type = OPType_ORDER_BY;
    orderBy->op.consume = OrderByConsume;
    orderBy->op.reset = OrderByReset;
    orderBy->op.free = OrderByFree;

    return (OpBase*)orderBy;
}

/* `op` is an actual variable in the caller function. Using it in a
 * macro like this is rather ugly, but the macro passed to QSORT must
 * accept only 2 arguments. */
#define RECORD_SORT(a, b) (_record_islt((*a), (*b), op))

/* OrderBy next operation
 * called each time a new ID is required */
OpResult OrderByConsume(OpBase *opBase, Record *r) {
    OrderBy *op = (OrderBy*) opBase;
    
    Record *record = _handoff(op);
    if(record != NULL) {
        *r = *record;
        return OP_OK;
    }

    // If we're here, we don't have any records to return
    // try to get records.
    OpBase *child = op->op.children[0];
    bool newData = false;
    while(child->consume(child, r) == OP_OK) {
        // TODO: Find a better approach to multiple individual allocations
        Record *clone = rm_malloc(sizeof(Record));
        Record_Clone(*r, clone);
        _accumulate(op, clone);
        newData = true;
    }
    if(!newData) return OP_DEPLETED;

    if(op->buffer) {
        QSORT(Record*, op->buffer, array_len(op->buffer), RECORD_SORT);
    } else {
        // Heap, responses need to be reversed.
        int record_idx = 0;
        int records_count = heap_count(op->heap);
        op->buffer = array_new(Record*, records_count);

        /* Pop items from heap */
        while(records_count > 0) {
            Record *r = heap_poll(op->heap);
            op->buffer = array_append(op->buffer, r);
            records_count--;
        }
    }

    // Pass ordered records downward.
    record = _handoff(op);
    if(record == NULL) return OP_DEPLETED;

    *r = *record;
    return OP_OK;
}

/* Restart iterator */
OpResult OrderByReset(OpBase *ctx) {
    OrderBy *op = (OrderBy*)ctx;
    uint recordCount;

    if(op->heap) {
        recordCount = heap_count(op->heap);
        for(uint i = 0; i < recordCount; i++) {
            Record r = (Record)heap_poll(op->heap);
            Record_Free(r);
        }
    }

    if(op->buffer) {
        recordCount = array_len(op->buffer);
        for(uint i = 0; i < recordCount; i++) {
            Record *r = array_pop(op->buffer);
            Record_Free(*r);
        }
    }
    return OP_OK;
}

/* Frees OrderBy */
void OrderByFree(OpBase *ctx) {
    OrderBy *op = (OrderBy*)ctx;
    uint recordCount;

    if(op->heap) heap_free(op->heap);

    // TODO: Either free allocated records
    // or change the way records are created/passed.
    if(op->buffer) {
        // recordCount = array_len(op->buffer);
        // for(uint i = 0; i < recordCount; i++) {
        //     Record *r = array_pop(op->buffer);
        //     Record_Free(*r);
        // }
        array_free(op->buffer);
    }

    for(int i = 0; i < array_len(op->fields); i++) {
        AR_EXP_Free(op->fields[i]);
    }
    array_free(op->fields);
}

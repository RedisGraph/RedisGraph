/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_hash_join.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include <assert.h>

#define NO_INTERSECTION -1

/* Determins order between two records by inspecting
 * node stored at postion intersect_node_idx. */
static bool _record_islt(Record l, Record r, uint intersect_node_idx) {
    Node *ln = Record_GetNode(l, intersect_node_idx);
    Node *rn = Record_GetNode(r, intersect_node_idx);
    return ENTITY_GET_ID(ln) < ENTITY_GET_ID(rn);
}

/* `intersect_node_idx` is an actual variable in the caller function.
 * Using it in a macro like this is rather ugly,
 * but the macro passed to QSORT must accept only 2 arguments. */
#define RECORD_SORT_ON_ENTRY(a, b) (_record_islt((*a), (*b), intersect_node_idx))

/* Retrive the next intersecting record
 * if such exists, otherwise returns NULL. */
static Record _get_intersecting_record(OpNodeHashJoin *op) {
    // No more intersecting records.
    if(op->intersect_idx == NO_INTERSECTION ||
       op->number_of_intersections == 0) return NULL;

    Record cr = op->cached_records[op->intersect_idx];
    
    // Update intersection trackers.
    op->intersect_idx++;
    op->number_of_intersections--;

    return cr;
}

static uint64_t _binarySearch(const OpNodeHashJoin *op, NodeID id) {
    Record *array = op->cached_records;
    uint32_t recordCount = array_len(array);
    uint32_t pos;
    uint32_t left = 0;
    uint32_t right = recordCount;
    while(left < right) {
        pos = (right + left) / 2;
        Node *n = Record_GetNode(array[pos], op->intersect_node_idx);
        if(ENTITY_GET_ID(n) < id) left = pos + 1;
        else right = pos;
    }
    return left;
}

/* Look up first cached record CR position
 * where CR node ID = id.
 * Returns NO_INTERSECTION if no such CR is found. */
int _set_intersection_idx(OpNodeHashJoin *op, NodeID id) {
    op->intersect_idx = NO_INTERSECTION;
    op->number_of_intersections = 0;
    int idx = _binarySearch(op, id);

    // Make sure node was found.
    Record r = op->cached_records[idx];
    Node *n = Record_GetNode(r, op->intersect_node_idx);

    // Node was not found.
    if(ENTITY_GET_ID(n) != id) return NO_INTERSECTION;

    /* Node was found, make sure op->intersect_idx
     * points to the first record holding node, 
     * and update number_of_intersections to count how many
     * records share the same node ID. */
    while(idx >= 0) {
        r = op->cached_records[idx];
        Node *n = Record_GetNode(r, op->intersect_node_idx);
        if(ENTITY_GET_ID(n) != id) break;
        idx--;
    }

    op->intersect_idx = idx;
    if(idx > 0) op->intersect_idx++;
    idx = op->intersect_idx;

    r = op->cached_records[idx];
    n = Record_GetNode(r, op->intersect_node_idx);
    uint record_count = array_len(op->cached_records);

    // Count how many records share the same node.
    while(idx < record_count) {
        r = op->cached_records[idx];
        n = Record_GetNode(r, op->intersect_node_idx);
        if(ENTITY_GET_ID(n) != id) break;
        op->number_of_intersections++;
    }

    return op->intersect_idx;
}

/* Caches all records coming from left branch. */
void _cache_records(OpNodeHashJoin *op) {
    assert(op->cached_records == NULL);

    OpBase *left_child = op->op.children[0];
    op->cached_records = array_new(Record, 32);

    Record l = NULL;
    while((l = left_child->consume(left_child)))
        op->cached_records = array_append(op->cached_records, l);
}

void _sort_cached_records(OpNodeHashJoin *op) {
    uint intersect_node_idx = op->intersect_node_idx;
    QSORT(Record, op->cached_records,
          array_len(op->cached_records), RECORD_SORT_ON_ENTRY);
}

static int NodeHashJoinToString(const OpBase *ctx, char *buff, uint buff_len) {
    const OpNodeHashJoin *op = (const OpNodeHashJoin*)ctx;

    int offset = 0;    
    offset += snprintf(buff + offset, buff_len-offset, "%s | ", op->op.name);
    offset += Node_ToString(op->intersect_node, buff + offset, buff_len - offset);
    return offset;
}

/* Creates a new NodeHashJoin operation */
OpBase *NewNodeHashJoin(const AST *ast, const Node *intersect_node) {
    OpNodeHashJoin *nodeHashJoin = malloc(sizeof(OpNodeHashJoin));
    nodeHashJoin->r = NULL;
    nodeHashJoin->intersect_idx = NO_INTERSECTION;
    nodeHashJoin->cached_records = NULL;
    nodeHashJoin->number_of_intersections = 0;
    nodeHashJoin->intersect_node = intersect_node;
    nodeHashJoin->intersect_node_idx = AST_GetAliasID(ast, intersect_node->alias);

    // Set our Op operations
    OpBase_Init(&nodeHashJoin->op);
    nodeHashJoin->op.name = "Index Scan";
    nodeHashJoin->op.type = OPType_INDEX_SCAN;
    nodeHashJoin->op.free = NodeHashJoinFree;
    nodeHashJoin->op.init = NodeHashJoinInit;
    nodeHashJoin->op.reset = NodeHashJoinReset;
    nodeHashJoin->op.consume = NodeHashJoinConsume;
    nodeHashJoin->op.toString = NodeHashJoinToString;

    nodeHashJoin->op.modifies = NULL;
    
    return (OpBase*)nodeHashJoin;    
}

OpResult NodeHashJoinInit(OpBase *ctx) {
    assert(ctx->childCount == 2);
    return OP_OK;
}

/* */
Record NodeHashJoinConsume(OpBase *opBase) {
    OpNodeHashJoin *op = (OpNodeHashJoin*)opBase;
    OpBase *right_child = op->op.children[1];

    // Eager, pull from left branch until depleted.
    if(op->cached_records == NULL) {
        _cache_records(op);
        // Sort cache on intersect node ID.
        _sort_cached_records(op);
    }

    /* Try to produce a record:
     * given a right hand side record R,
     * and an intersection index:
     * cached_records[intersect_idx] intersects with R
     * return merged record:
     * cached_records[intersect_idx] merged with R. */

    Record l;
    if(op->intersect_idx != NO_INTERSECTION) {
        while((l = _get_intersecting_record(op))) {
            Record_Merge(&op->r, l);
            return Record_Clone(op->r);
        }
    }

    /* If we're here there are no more
     * left hand side records which intersect with R
     * discard R. */
    if(op->r) Record_Free(op->r);

    /* Try to get new right hand side record
     * which intersect with a left hand side record. */
    while(true) {
        // Pull from right branch.
        Record r = right_child->consume(right_child);
        if(!r) return NULL;

        // Get node on which we're intersecting.
        Node *n = Record_GetNode(r, op->intersect_node_idx);
        
        // No intersection, discard R.
        if(_set_intersection_idx(op, ENTITY_GET_ID(n)) == NO_INTERSECTION) {
            Record_Free(r);
            continue;
        }

        // Found atleast one intersecting record.
        op->r = r;
        l = _get_intersecting_record(op);
        Record_Merge(&op->r, l);
        return Record_Clone(op->r);
    }
}

/* */
OpResult NodeHashJoinReset(OpBase *ctx) {
    OpNodeHashJoin *op = (OpNodeHashJoin*)ctx;
    op->number_of_intersections = 0;
    op->intersect_idx = NO_INTERSECTION;

    // Clear cached records.
    uint record_count = array_len(op->cached_records);
    for(uint i = 0; i < record_count; i++) {
        Record r = op->cached_records[i];
        Record_Free(r);
    }
    array_free(op->cached_records);
    op->cached_records = NULL;

    return OP_OK;
}

/* Frees NodeHashJoin */
void NodeHashJoinFree(OpBase *ctx) {
    OpNodeHashJoin *op = (OpNodeHashJoin*)ctx;
    // Free cached records.
    uint record_count = array_len(op->cached_records);
    for(uint i = 0; i < record_count; i++) {
        Record r = op->cached_records[i];
        Record_Free(r);
    }
    array_free(op->cached_records);
}

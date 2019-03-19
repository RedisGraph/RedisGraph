/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../util/arr.h"

static inline PropertyTypeUser _mapValueType(const SIValue v) {
    switch (SI_TYPE(v)) {
        case T_NULL:
            return PROPERTY_NULL;
        case T_STRING:
            return PROPERTY_STRING;
        case T_INT64:
            return PROPERTY_INTEGER;
        case T_BOOL:
            return PROPERTY_BOOLEAN;
        case T_DOUBLE:
            return PROPERTY_DOUBLE;
        default:
            return PROPERTY_UNKNOWN;
    }
}

static inline void _ResultSet_ReplyWithValueType(RedisModuleCtx *ctx, const SIValue v) {
    RedisModule_ReplyWithLongLong(ctx, _mapValueType(v));
}


void ResultSet_CompactReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc, const SIValue v) {
    _ResultSet_ReplyWithValueType(ctx, v);
    // Emit the actual value, then the value type (to facilitate client-side parsing)
    switch (SI_TYPE(v)) {
        case T_STRING:
            RedisModule_ReplyWithStringBuffer(ctx, v.stringval, strlen(v.stringval));
            return;
        case T_INT64:
            RedisModule_ReplyWithLongLong(ctx, v.longval);
            return;
        case T_DOUBLE:
            _ResultSet_ReplyWithRoundedDouble(ctx, v.doubleval);
            return;
        case T_BOOL:
            if (v.longval != 0) RedisModule_ReplyWithStringBuffer(ctx, "true", 4);
            else RedisModule_ReplyWithStringBuffer(ctx, "false", 5);
            return;
        case T_NULL:
            RedisModule_ReplyWithNull(ctx);
            return;
        case T_NODE: // Nodes and edges should always be Record entries at this point
        case T_EDGE:
        default:
            assert("Unhandled value type" && false);
      }
}

static void _ResultSet_CompactReplyWithProperties(RedisModuleCtx *ctx, GraphContext *gc, const GraphEntity *e) {
    int prop_count = ENTITY_PROP_COUNT(e);
    RedisModule_ReplyWithArray(ctx, prop_count);
    // Iterate over all properties stored on entity
    for (int i = 0; i < prop_count; i ++) {
        // Compact replies include the value's type; verbose replies do not
        RedisModule_ReplyWithArray(ctx, 3);
        EntityProperty prop = ENTITY_PROPS(e)[i];
        // Emit the string offset
        RedisModule_ReplyWithLongLong(ctx, prop.id);
        // Emit the value
        ResultSet_CompactReplyWithSIValue(ctx, gc, prop.value);
    }
}

static void _ResultSet_CompactReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n) {
    /*  Compact node reply format:
     *  [
     *      Node ID (integer),
            [label string offset (integer)],
     *      [[name, value, value type] X N]
     *  ]
     */
    // 3 top-level entities in node reply
    RedisModule_ReplyWithArray(ctx, 3);

    // id (integer)
    EntityID id = ENTITY_GET_ID(n);
    RedisModule_ReplyWithLongLong(ctx, id);

    // [label string offset]
    // Print label in nested array for multi-label support
    // Retrieve label
    const char *label = GraphContext_GetNodeLabel(gc, n);
    int label_id = Graph_GetNodeLabel(gc->g, id);
    if (label == NULL) {
        // Emit an empty array for unlabeled nodes
        RedisModule_ReplyWithArray(ctx, 0);
    } else {
        RedisModule_ReplyWithArray(ctx, 1);
        // TODO Unsafe if the query has extended the string mapping
        int offset = array_len(gc->string_mapping) + label_id;
        RedisModule_ReplyWithLongLong(ctx, offset);
    }

    // [properties]
    _ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity*)n);
}

static void _ResultSet_CompactReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e) {
    /*  Compact edge reply format:
     *  [
     *      Edge ID (integer),
            reltype string offset (integer),
            src node ID offset (integer),
            dest node ID offset (integer),
     *      [[name, value, value type] X N]
     *  ]
     */
    // 5 top-level entities in edge reply
    RedisModule_ReplyWithArray(ctx, 5);

    // id (integer)
    EntityID id = ENTITY_GET_ID(e);
    RedisModule_ReplyWithLongLong(ctx, id);

    // reltype string offset
    // Retrieve reltype
    int reltype_id = Graph_GetEdgeRelation(gc->g, e);
    assert(reltype_id != GRAPH_NO_RELATION);

    // src node ID
    RedisModule_ReplyWithLongLong(ctx, Edge_GetSrcNodeID(e));

    // dest node ID
    RedisModule_ReplyWithLongLong(ctx, Edge_GetDestNodeID(e));

    // TODO Unsafe if the query has extended the string mapping
    int offset = array_len(gc->string_mapping) + array_len(gc->node_schemas) + reltype_id;
    RedisModule_ReplyWithLongLong(ctx, offset);

    // [properties]
    _ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity*)e);
}

void ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols) {
    // Prepare return array sized to the number of RETURN entities
    RedisModule_ReplyWithArray(ctx, numcols);

    for (uint i = 0; i < numcols; i++) {
        switch (Record_GetType(r, i)) {
            case REC_TYPE_NODE:
                _ResultSet_CompactReplyWithNode(ctx, gc, Record_GetNode(r, i));
                break;
            case REC_TYPE_EDGE:
                _ResultSet_CompactReplyWithEdge(ctx, gc, Record_GetEdge(r, i));
                break;
            default:
                RedisModule_ReplyWithArray(ctx, 2); // Reply with array with space for type and value
                ResultSet_CompactReplyWithSIValue(ctx, gc, Record_GetScalar(r, i));
        }
    }
}

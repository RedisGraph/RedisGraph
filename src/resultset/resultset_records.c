/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_records.h"
#include "../value.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../util/rmalloc.h"
#include "../util/arr.h"
#include <assert.h>

/* Redis prints doubles with up to 17 digits of precision, which captures
 * the inaccuracy of many floating-point numbers (such as 0.1).
 * By using the %g format and a precision of 15 significant digits, we avoid many
 * awkward representations like RETURN 0.1 emitting "0.10000000000000001",
 * though we're still subject to many of the typical issues with floating-point error. */
static inline void _ResultSet_ReplyWithRoundedDouble(RedisModuleCtx *ctx, double d) {
    // Get length required to print number
    int len = snprintf(NULL, 0, "%.15g", d);
    char str[len + 1]; // TODO a reusable buffer would be far preferable
    sprintf(str, "%.15g", d);
    // Output string-formatted number
    RedisModule_ReplyWithStringBuffer(ctx, str, len);
}

static void _ResultSet_ReplyWithProperties(RedisModuleCtx *ctx, GraphContext *gc, const GraphEntityType t, const GraphEntity *e, bool compact) {
    int prop_count = ENTITY_PROP_COUNT(e);
    RedisModule_ReplyWithArray(ctx, prop_count);
    // Iterate over all properties stored on entity
    for (int i = 0; i < prop_count; i ++) {
        // Compact replies include the value's type; verbose replies do not
        int reply_len = (compact) ? 3 : 2;
        RedisModule_ReplyWithArray(ctx, reply_len);
        EntityProperty prop = ENTITY_PROPS(e)[i];
        if (compact) {
            // Emit the string offset
            RedisModule_ReplyWithLongLong(ctx, prop.id);
        } else {
            // Emit the actual string
            const char *prop_str = GraphContext_GetAttributeString(gc, prop.id);
            RedisModule_ReplyWithStringBuffer(ctx, prop_str, strlen(prop_str));
        }
        // Emit the value
        ResultSet_ReplyWithSIValue(ctx, gc, prop.value, compact);
    }
}

static void _ResultSet_VerboseReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n) {
    /*  Verbose node reply format:
     *  [
     *      ["id", Node ID (integer)]
     *      ["label", [label (string or NULL)]]
     *      ["properties", [[name, value, value type] X N]
     *  ]
     */
    // 3 top-level entities in node reply
    RedisModule_ReplyWithArray(ctx, 3);

    // ["id", id (integer)]
    EntityID id = ENTITY_GET_ID(n);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
    RedisModule_ReplyWithLongLong(ctx, id);

    // ["labels", [label (string)]]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "labels", 6);
    // Print label in nested array for multi-label support
    // Retrieve label
    // TODO Make a more efficient lookup for this string
    const char *label = GraphContext_GetNodeLabel(gc, n);
    if (label == NULL) {
        // Emit an empty array for unlabeled nodes
        RedisModule_ReplyWithArray(ctx, 0);
    } else {
        RedisModule_ReplyWithArray(ctx, 1);
        RedisModule_ReplyWithStringBuffer(ctx, label, strlen(label));
    }

    // [properties, [properties]]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "properties", 10);
    _ResultSet_ReplyWithProperties(ctx, gc, GETYPE_NODE, (GraphEntity*)n, false);
}

// static void _ResultSet_ReplyWithLabelOffset(RedisModuleCtx *ctx, GraphContext *gc)

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
        // TODO probably unsafe
        int offset = array_len(gc->string_mapping) + label_id;
        RedisModule_ReplyWithLongLong(ctx, offset);
    }

    // [properties]
    _ResultSet_ReplyWithProperties(ctx, gc, GETYPE_NODE, (GraphEntity*)n, true);
}

static void _ResultSet_VerboseReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e) {
    /*  Edge reply format:
     *  [
     *      ["id", Edge ID (integer)]
     *      ["type", relation type (string)]
     *      ["src_node", source node ID (integer)]
     *      ["dest_node", destination node ID (integer)]
     *      ["properties", [[name, value, value type] X N]
     *  ]
     */
    // 5 top-level entities in edge reply
    RedisModule_ReplyWithArray(ctx, 5);

    // ["id", id (integer)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
    RedisModule_ReplyWithLongLong(ctx, ENTITY_GET_ID(e));

    // ["type", type (string)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "type", 4);
    // Retrieve relation type
    // TODO Make a more efficient lookup for this string
    const char *reltype = GraphContext_GetEdgeRelationType(gc, e);
    RedisModule_ReplyWithStringBuffer(ctx, reltype, strlen(reltype));

    // ["src_node", srcNodeID (integer)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "src_node", 8);
    RedisModule_ReplyWithLongLong(ctx, Edge_GetSrcNodeID(e));

    // ["dest_node", destNodeID (integer)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "dest_node", 9);
    RedisModule_ReplyWithLongLong(ctx, Edge_GetDestNodeID(e));

    // [properties, [properties]]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "properties", 10);
    _ResultSet_ReplyWithProperties(ctx, gc, GETYPE_EDGE, (GraphEntity*)e, false);
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
    // TODO probably unsafe
    int offset = array_len(gc->string_mapping) + array_len(gc->node_schemas) + reltype_id;
    RedisModule_ReplyWithLongLong(ctx, offset);

    // [properties]
    _ResultSet_ReplyWithProperties(ctx, gc, GETYPE_NODE, (GraphEntity*)e, true);
}

static inline PropertyTypeUser _mapValueType(const SIValue v) {
    // TODO does this abstraction make sense? Is it overkill,
    // should we include not-yet-supported types like arrays and paths?
    switch (SI_TYPE(v)) {
        case T_NULL:
            return PROPERTY_NULL;
        case T_STRING:
            return PROPERTY_STRING_OFFSET;
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

/* This function has handling for all SIValue scalar types.
 * The current RESP protocol only has unique support for strings, 8-byte integers,
 * and NULL values. */
void ResultSet_ReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc, const SIValue v, bool print_type) {
    if (print_type) _ResultSet_ReplyWithValueType(ctx, v);
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

void _ResultSet_EmitVerboseRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols) {
    // Prepare return array sized to the number of RETURN entities
    RedisModule_ReplyWithArray(ctx, numcols);

    for(int i = 0; i < numcols; i++) {
        switch (Record_GetType(r, i)) {
            case REC_TYPE_NODE:
                _ResultSet_VerboseReplyWithNode(ctx, gc, Record_GetNode(r, i));
                break;
            case REC_TYPE_EDGE:
                _ResultSet_VerboseReplyWithEdge(ctx, gc, Record_GetEdge(r, i));
                break;
            default:
                ResultSet_ReplyWithSIValue(ctx, gc, Record_GetScalar(r, i), false);
        }
    }
}

void _ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols) {
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
                ResultSet_ReplyWithSIValue(ctx, gc, Record_GetScalar(r, i), true);
        }
    }
}

EmitRecordFunc ResultSet_SetReplyFormatter(bool compact) {
    if (compact) return _ResultSet_EmitCompactRecord;
    return _ResultSet_EmitVerboseRecord;
}


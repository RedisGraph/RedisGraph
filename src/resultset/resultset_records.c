/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "resultset_records.h"
#include "../value.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
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

static void _ResultSet_ReplyWithProperties(RedisModuleCtx *ctx, const GraphEntity *e) {
    int prop_count = ENTITY_PROP_COUNT(e);
    RedisModule_ReplyWithArray(ctx, prop_count);
    // Iterate over all properties stored on entity
    for (int i = 0; i < prop_count; i ++) {
        RedisModule_ReplyWithArray(ctx, 2);
        EntityProperty prop = ENTITY_PROPS(e)[i];
        // Emit the string key
        // TODO get property names
        // RedisModule_ReplyWithStringBuffer(ctx, prop.name, strlen(prop.name));
        RedisModule_ReplyWithStringBuffer(ctx, "", 1);
        // Emit the value
        ResultSet_ReplyWithSIValue(ctx, prop.value, false);
    }
}

static void _ResultSet_ReplyWithNode(RedisModuleCtx *ctx, Node *n) {
    /*  Node reply format:
     *  [
     *      ["type", "node"]
     *      ["id", Node ID (integer)]
     *      ["label", [label (string or NULL)]]
     *      ["properties", [[name, value, value type] X N]
     *  ]
     */
    // 4 top-level entities in node reply
    RedisModule_ReplyWithArray(ctx, 4);

    // ["type", "node"]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "type", 4);
    RedisModule_ReplyWithStringBuffer(ctx, "node", 4);

    // ["id", id (integer)]
    int id = ENTITY_GET_ID(n);
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
    RedisModule_ReplyWithLongLong(ctx, id);

    // ["labels", [label (string)]]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "labels", 6);
    // Print label in nested array for multi-label support
    RedisModule_ReplyWithArray(ctx, 1);
    // Retrieve label
    // TODO Make a more efficient lookup for this string
    GraphContext *gc = GraphContext_GetFromTLS();
    const char *label = GraphContext_GetNodeLabel(gc, n);
    if (label == NULL) {
        RedisModule_ReplyWithNull(ctx);
    } else {
        RedisModule_ReplyWithStringBuffer(ctx, label, strlen(label));
    }

    // [properties, [properties]]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "properties", 10);
    _ResultSet_ReplyWithProperties(ctx, (GraphEntity*)n);
}

static void _ResultSet_ReplyWithEdge(RedisModuleCtx *ctx, Edge *e) {
    /*  Edge reply format:
     *  [
     *      ["type", "relation"]
     *      ["id", Edge ID (integer)]
     *      ["relation_type", relation type (string)]
     *      ["src_node", source node ID (integer)]
     *      ["dest_node", destination node ID (integer)]
     *      ["properties", [[name, value, value type] X N]
     *  ]
     */
    // 6 top-level entities in node reply
    RedisModule_ReplyWithArray(ctx, 6);

    // ["type", "relation"]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "type", 4);
    RedisModule_ReplyWithStringBuffer(ctx, "relation", 8);

    // ["id", id (integer)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
    RedisModule_ReplyWithLongLong(ctx, ENTITY_GET_ID(e));

    // ["relation_type", type (string)]
    RedisModule_ReplyWithArray(ctx, 2);
    RedisModule_ReplyWithStringBuffer(ctx, "relation_type", 13);
    // Retrieve relation type
    // TODO Make a more efficient lookup for this string
    GraphContext *gc = GraphContext_GetFromTLS();
    const char *label = GraphContext_GetEdgeRelationType(gc, e);
    RedisModule_ReplyWithStringBuffer(ctx, label, strlen(label));

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
    _ResultSet_ReplyWithProperties(ctx, (GraphEntity*)e);
}

/* This function has handling for all SIValue scalar types.
 * The current RESP protocol only has unique support for strings, 8-byte integers,
 * and NULL values. */
void ResultSet_ReplyWithSIValue(RedisModuleCtx *ctx, const SIValue v, bool print_type) {
    // Emit the actual value, then the value type (to facilitate client-side parsing)
    switch (SI_TYPE(v)) {
        case T_NODE:
           _ResultSet_ReplyWithNode(ctx, (Node*)v.ptrval);
           return;
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
        default:
            assert("Unhandled value type" && false);
      }
}

void ResultSet_EmitRecord(RedisModuleCtx *ctx, const Record r, unsigned int numcols) {
    // Prepare return array sized to the number of RETURN entities
    RedisModule_ReplyWithArray(ctx, numcols);

    for(int i = 0; i < numcols; i++) {
        switch (Record_GetType(r, i)) {
            case REC_TYPE_NODE:
                _ResultSet_ReplyWithNode(ctx, Record_GetNode(r, i));
                break;
            case REC_TYPE_EDGE:
                _ResultSet_ReplyWithEdge(ctx, Record_GetEdge(r, i));
                break;
            default:
                ResultSet_ReplyWithSIValue(ctx, Record_GetScalar(r, i), false);
        }
    }
}


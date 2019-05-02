/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "../../util/arr.h"
#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

OpBase* NewCreateOp(ResultSet *result_set, NodeCreateCtx *nodes, EdgeCreateCtx *edges, uint record_len) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = GraphContext_GetFromTLS();
    op_create->records = NULL;
    op_create->record_len = record_len;
    op_create->nodes_to_create = nodes;
    op_create->edges_to_create = edges;
    op_create->created_nodes = array_new(Node*, 0);
    op_create->created_edges = array_new(Edge*, 0);
    op_create->node_properties = array_new(PropertyMap*, 0);
    op_create->edge_properties = array_new(PropertyMap*, 0);
    op_create->result_set = result_set;

    // TODO modified?

    // Set our Op operations
    OpBase_Init(&op_create->op);
    op_create->op.name = "Create";
    op_create->op.type = OPType_CREATE;
    op_create->op.consume = OpCreateConsume;
    op_create->op.reset = OpCreateReset;
    op_create->op.free = OpCreateFree;

    return (OpBase*)op_create;
}

// TODO improve, consolidate, etc
void _AddNodeProperties(OpCreate *op, Schema *schema, Node *n, PropertyMap *props) {
    if (props == NULL) return;

    GraphContext *gc = op->gc;
    Attribute_ID prop_id = ATTRIBUTE_NOTFOUND;

    for(int i = 0; i < props->property_count; i++) {
        prop_id = Schema_AddAttribute(schema, SCHEMA_NODE, props->keys[i]);
        GraphEntity_AddProperty((GraphEntity*)n, prop_id, props->values[i]);
    }

    op->result_set->stats.properties_set += props->property_count;
}

void _AddEdgeProperties(OpCreate *op, Schema *schema, Edge *e, PropertyMap *props) {
    if (props == NULL) return;

    GraphContext *gc = op->gc;
    Attribute_ID prop_id = ATTRIBUTE_NOTFOUND;

    for(int i = 0; i < props->property_count; i++) {
        prop_id = Schema_AddAttribute(schema, SCHEMA_EDGE, props->keys[i]);
        GraphEntity_AddProperty((GraphEntity*)e, prop_id, props->values[i]);
    }

    op->result_set->stats.properties_set += props->property_count;
}

void _CreateNodes(OpCreate *op, Record r) {
    uint nodes_to_create_count = array_len(op->nodes_to_create);
    for(uint i = 0; i < nodes_to_create_count; i++) {
        /* Get specified node to create. */
        Node *n = op->nodes_to_create[i].node;

        /* Create a new node. */
        Node *newNode = Node_New(n->label, n->alias);

        /* Save node for later insertion. */
        op->created_nodes = array_append(op->created_nodes, newNode);

        /* Save reference to property map */
        op->node_properties = array_append(op->node_properties, op->nodes_to_create[i].properties);

        /* Update record with new node. */
        Record_AddScalar(r, op->nodes_to_create[i].node_idx, SI_PtrVal(newNode));
    }
}

void _CreateEdges(OpCreate *op, Record r) {
    uint edges_to_create_count = array_len(op->edges_to_create);
    for(uint i = 0; i < edges_to_create_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].src_idx);
        Node *dest_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].dest_idx);

        /* Create the actual edge. */
        Edge *newEdge = Edge_New(src_node, dest_node, e->relationship, e->alias);

        /* Save edge for later insertion. */
        op->created_edges = array_append(op->created_edges, newEdge);

        /* Save reference to property map */
        op->edge_properties = array_append(op->edge_properties, op->edges_to_create[i].properties);

        /* Update record with new edge. */
        Record_AddScalar(r, op->edges_to_create[i].edge_idx, SI_PtrVal(newEdge));
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op) {
    Node *n;
    int labelID;
    Graph *g = op->gc->g;
    Schema *unified_schema = GraphContext_GetUnifiedSchema(op->gc, SCHEMA_NODE);

    uint node_count = array_len(op->created_nodes);
    Graph_AllocateNodes(op->gc->g, node_count);


    for(uint i = 0; i < node_count; i++) {
        n = op->created_nodes[i];
        Schema *schema = NULL;

        // Get label ID.
        if(n->label == NULL) {
            labelID = GRAPH_NO_LABEL;
            schema = unified_schema;
        } else {
            schema = GraphContext_GetSchema(op->gc, n->label, SCHEMA_NODE);
            if(schema == NULL) {
                schema = GraphContext_AddSchema(op->gc, n->label, SCHEMA_NODE);
                op->result_set->stats.labels_added++;
            }
            labelID = schema->id;
        }

        // Introduce node into graph.
        Graph_CreateNode(g, labelID, n);

        _AddNodeProperties(op, schema, n, op->node_properties[i]);

        if(n->label) GraphContext_AddNodeToIndices(op->gc, schema, n);
    }

}

static void _CommitEdges(OpCreate *op) {
    Edge *e;
    Graph *g = op->gc->g;
    int relationships_created = 0;

    uint edge_count = array_len(op->created_edges);
    for(uint i = 0; i < edge_count; i++) {
        e = op->created_edges[i];
        NodeID srcNodeID;

        // Nodes which already existed prior to this query would
        // have their ID set under e->srcNodeID and e->destNodeID
        // Nodes which are created as part of this query would be
        // saved under edge src/dest pointer.
        if(e->srcNodeID != INVALID_ENTITY_ID) srcNodeID = e->srcNodeID;
        else srcNodeID = ENTITY_GET_ID(Edge_GetSrcNode(e));

        NodeID destNodeID;
        if(e->destNodeID != INVALID_ENTITY_ID) destNodeID = e->destNodeID;
        else destNodeID = ENTITY_GET_ID(Edge_GetDestNode(e));

        Schema *schema = GraphContext_GetSchema(op->gc, e->relationship, SCHEMA_EDGE);
        if(!schema) schema = GraphContext_AddSchema(op->gc, e->relationship, SCHEMA_EDGE);
        int relation_id = schema->id;

        if(!Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e)) continue;
        relationships_created++;

        // Set edge properties.
        _AddEdgeProperties(op, schema, e, op->edge_properties[i]);
    }

    op->result_set->stats.relationships_created += relationships_created;
}

static void _CommitNewEntities(OpCreate *op) {
    size_t node_count = array_len(op->created_nodes);
    size_t edge_count = array_len(op->created_edges);

    // Lock everything.
    Graph_AcquireWriteLock(op->gc->g);

    if(node_count > 0) {
        _CommitNodes(op);
        op->result_set->stats.nodes_created += node_count;
    }

    if(edge_count > 0) _CommitEdges(op);

    // Release lock.
    Graph_ReleaseLock(op->gc->g);
}

static Record _handoff(OpCreate *op) {
    Record r = NULL;
    if(array_len(op->records)) r = array_pop(op->records);
    return r;
}

Record OpCreateConsume(OpBase *opBase) {
    OpCreate *op = (OpCreate*)opBase;
    Record r;

    // Return mode, all data was consumed.
    if(op->records) return _handoff(op);

    // Consume mode.
    op->records = array_new(Record, 32);

    // No child operation to call.
    if(!op->op.childCount) {
        r = Record_New(op->record_len);
        /* Create entities. */
        _CreateNodes(op, r);
        _CreateEdges(op, r);

        // Save record for later use.
        op->records = array_append(op->records, r);
    } else {
        // Pull data until child is depleted.
        OpBase *child = op->op.children[0];
        while((r = child->consume(child))) {
            /* Create entities. */
            _CreateNodes(op, r);
            _CreateEdges(op, r);

            // Save record for later use.
            op->records = array_append(op->records, r);
        }
    }

    // Create entities.
    _CommitNewEntities(op);

    // Return record.
    return _handoff(op);
}

OpResult OpCreateReset(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    return OP_OK;
}

void OpCreateFree(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    if(op->records) {
        assert(array_len(op->records) == 0);
        array_free(op->records);
    }
    if(op->nodes_to_create) {
        uint nodes_to_create_count = array_len(op->nodes_to_create);
        for (uint i = 0; i < nodes_to_create_count; i ++) {
            PropertyMap_Free(op->nodes_to_create[i].properties);
        }
        array_free(op->nodes_to_create);
    }

    if(op->edges_to_create) {
        uint edges_to_create_count = array_len(op->edges_to_create);
        for (uint i = 0; i < edges_to_create_count; i ++) {
            PropertyMap_Free(op->edges_to_create[i].properties);
        }
        array_free(op->edges_to_create);
    }

    if (op->created_nodes) {
        size_t nodeCount = array_len(op->created_nodes);
        for(uint i = 0; i < nodeCount; i++) {
            // It's safe to free node, as its internal GraphEntity wouldn't be free.
            Node_Free(op->created_nodes[i]);
        }
        array_free(op->created_nodes);
        array_free(op->node_properties);
    }

    if (op->created_edges) {
        size_t edgeCount = array_len(op->created_edges);
        for(uint i = 0; i < edgeCount; i++) {
            // It's safe to free edge, as its internal GraphEntity wouldn't be free.
            Edge_Free(op->created_edges[i]);
        }
        array_free(op->created_edges);
        array_free(op->edge_properties);
    }
}

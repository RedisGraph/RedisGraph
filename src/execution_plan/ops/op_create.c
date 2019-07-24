/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "../../util/arr.h"
#include "../../schema/schema.h"
#include <assert.h>

OpBase* NewCreateOp(ResultSetStatistics *stats, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = GraphContext_GetFromTLS();
    op_create->records = NULL;
    op_create->nodes_to_create = nodes;
    op_create->edges_to_create = edges;
    op_create->created_nodes = array_new(Node*, 0);
    op_create->created_edges = array_new(Edge*, 0);
    op_create->node_properties = array_new(PropertyMap*, 0);
    op_create->edge_properties = array_new(PropertyMap*, 0);
    op_create->stats = stats;

    // TODO modified?

    // Set our Op operations
    OpBase_Init(&op_create->op);
    op_create->op.name = "Create";
    op_create->op.type = OPType_CREATE;
    op_create->op.consume = OpCreateConsume;
    op_create->op.init = OpCreateInit;
    op_create->op.reset = OpCreateReset;
    op_create->op.free = OpCreateFree;

    return (OpBase*)op_create;
}

// TODO improve, consolidate, etc
static void _AddProperties(OpCreate *op, GraphEntity *ge, PropertyMap *props) {
    if (props == NULL) return;

    for(int i = 0; i < props->property_count; i++) {
        Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, props->keys[i]);
        GraphEntity_AddProperty(ge, prop_id, props->values[i]);
    }

    op->stats->properties_set += props->property_count;
}

void _CreateNodes(OpCreate *op, Record r) {
    uint nodes_to_create_count = array_len(op->nodes_to_create);
    for(uint i = 0; i < nodes_to_create_count; i++) {
        /* Get specified node to create. */
        QGNode *n = op->nodes_to_create[i].node;

        /* Create a new node. */
        Node *newNode = Record_GetNode(r, op->nodes_to_create[i].node_idx);
        newNode->entity = NULL;
        newNode->alias = n->alias;
        newNode->label = n->label;
        newNode->labelID = n->labelID;

        /* Save node for later insertion. */
        op->created_nodes = array_append(op->created_nodes, newNode);

        /* Save reference to property map */
        op->node_properties = array_append(op->node_properties, op->nodes_to_create[i].properties);
    }
}

void _CreateEdges(OpCreate *op, Record r) {
    uint edges_to_create_count = array_len(op->edges_to_create);
    for(uint i = 0; i < edges_to_create_count; i++) {
        /* Get specified edge to create. */
        QGEdge *e = op->edges_to_create[i].edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = Record_GetNode(r, op->edges_to_create[i].src_idx);
        Node *dest_node = Record_GetNode(r, op->edges_to_create[i].dest_idx);

        /* Create the actual edge. */
        Edge *newEdge = Record_GetEdge(r, op->edges_to_create[i].edge_idx);
        newEdge->alias = e->alias;
        if (array_len(e->reltypes) > 0) newEdge->relationship = e->reltypes[0];
        Edge_SetSrcNode(newEdge, src_node);
        Edge_SetDestNode(newEdge, dest_node);

        /* Save edge for later insertion. */
        op->created_edges = array_append(op->created_edges, newEdge);

        /* Save reference to property map */
        op->edge_properties = array_append(op->edge_properties, op->edges_to_create[i].properties);
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op) {
    Node *n;
    int labelID;
    Graph *g = op->gc->g;
    
    uint node_count = array_len(op->created_nodes);
    Graph_AllocateNodes(op->gc->g, node_count);

    for(uint i = 0; i < node_count; i++) {
        n = op->created_nodes[i];
        Schema *schema = NULL;

        // Get label ID.
        if(n->label == NULL) {
            labelID = GRAPH_NO_LABEL;
        } else {
            schema = GraphContext_GetSchema(op->gc, n->label, SCHEMA_NODE);
            if(schema == NULL) {
                schema = GraphContext_AddSchema(op->gc, n->label, SCHEMA_NODE);
                op->stats->labels_added++;
            }
            labelID = schema->id;
        }

        // Introduce node into graph.
        Graph_CreateNode(g, labelID, n);

        _AddProperties(op, (GraphEntity*)n, op->node_properties[i]);

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
        NodeID destNodeID;

        // Nodes which already existed prior to this query would
        // have their ID set under e->srcNodeID and e->destNodeID
        // Nodes which are created as part of this query would be
        // saved under edge src/dest pointer.
        if(e->srcNodeID != INVALID_ENTITY_ID) srcNodeID = e->srcNodeID;
        else srcNodeID = ENTITY_GET_ID(Edge_GetSrcNode(e));
        if(e->destNodeID != INVALID_ENTITY_ID) destNodeID = e->destNodeID;
        else destNodeID = ENTITY_GET_ID(Edge_GetDestNode(e));

        Schema *schema = GraphContext_GetSchema(op->gc, e->relationship, SCHEMA_EDGE);
        if(!schema) schema = GraphContext_AddSchema(op->gc, e->relationship, SCHEMA_EDGE);
        int relation_id = schema->id;

        if(!Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e)) continue;
        relationships_created++;

        // Set edge properties.
        _AddProperties(op, (GraphEntity*)e, op->edge_properties[i]);
    }

    op->stats->relationships_created += relationships_created;
}

static void _CommitNewEntities(OpCreate *op) {
    Graph *g = op->gc->g;

    // Lock everything.
    Graph_AcquireWriteLock(g);
    Graph_SetMatrixPolicy(g, RESIZE_TO_CAPACITY);
    uint node_count = array_len(op->created_nodes);
    if(node_count > 0) _CommitNodes(op, createEntities);
    if(array_len(op->created_edges) > 0) _CommitEdges(op, createEntities);
    Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);
    // Release lock.
    Graph_ReleaseLock(g);

    op->stats->nodes_created += node_count;
}

static Record _handoff(OpCreate *op) {
    Record r = NULL;
    if(array_len(op->records)) r = array_pop(op->records);
    return r;
}

OpResult OpCreateInit(OpBase *opBase) {
    return OP_OK;
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
        r = Record_New(opBase->record_map->record_len);
        /* Create entities. */
        _CreateNodes(op, r);
        _CreateEdges(op, r);

        // Save record for later use.
        op->records = array_append(op->records, r);
    } else {
        // Pull data until child is depleted.
        OpBase *child = op->op.children[0];
        while((r = OpBase_Consume(child))) {
            if (Record_length(r) < opBase->record_map->record_len) {
                // If the child record was created in a different segment, it may not be
                // large enough to accommodate the new entities.
                Record_Extend(&r, opBase->record_map->record_len);
            }
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
        uint rec_count = array_len(op->records);
        for(uint i = 0; i < rec_count; i++) Record_Free(op->records[i]);
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
        array_free(op->created_nodes);
        array_free(op->node_properties);
    }

    if (op->created_edges) {
        array_free(op->created_edges);
        array_free(op->edge_properties);
    }
}

/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "../../util/arr.h"
#include "../../parser/ast.h"
#include "../../schema/schema.h"

#include <assert.h>

// Determin which entities are modified by create op.
void _SetModifiedEntities(OpCreate *op) {
    AST *ast = op->ast;
    QueryGraph *qg = op->qg;
    size_t create_entity_count = qg->node_count + qg->edge_count;

    op->nodes_to_create = rm_malloc(sizeof(NodeCreateCtx) * create_entity_count);
    op->edges_to_create = rm_malloc(sizeof(EdgeCreateCtx) * create_entity_count);

    int node_idx = 0;
    int edge_idx = 0;

    /* For every entity within the CREATE clause see if it's also mentioned 
     * within the MATCH clause. */
    TrieMap *matchEntities = NewTrieMap();
    MatchClause_DefinedEntities(op->ast->matchNode, matchEntities);

    // Nodes.
    for(int i = 0; i < qg->node_count; i++) {
        Node *n = qg->nodes[i];
        char *alias = n->alias;
        
        if(TrieMap_Add(matchEntities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE) == 0) continue;
        op->nodes_to_create[node_idx].node = n;
        op->nodes_to_create[node_idx].node_rec_idx = AST_GetAliasID(ast, n->alias);
        node_idx++;
    }

    // Edges.
    for(int i = 0; i < qg->edge_count; i++) {
        Edge *e = qg->edges[i];
        char *alias = e->alias;
        if(TrieMap_Add(matchEntities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE) == 0) continue;

        op->edges_to_create[edge_idx].edge = e;
        op->edges_to_create[edge_idx].edge_rec_idx = AST_GetAliasID(ast, e->alias);
        op->edges_to_create[edge_idx].src_node_rec_idx = AST_GetAliasID(ast, e->src->alias);
        op->edges_to_create[edge_idx].dest_node_rec_idx = AST_GetAliasID(ast, e->dest->alias);
        edge_idx++;
    }

    TrieMap_Free(matchEntities, TrieMap_NOP_CB);

    op->node_count = node_idx;
    op->edge_count = edge_idx;
    /* Create must modify atleast one entity. */
    assert((op->node_count + op->edge_count) > 0);
}

OpBase* NewCreateOp(RedisModuleCtx *ctx, AST *ast, QueryGraph *qg, ResultSet *result_set) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = GraphContext_GetFromTLS();
    op_create->qg = qg;
    op_create->ast = ast;
    op_create->node_count = 0;
    op_create->edge_count = 0;
    op_create->records = NULL;
    op_create->nodes_to_create = NULL;
    op_create->edges_to_create = NULL;
    op_create->created_nodes = array_new(Node*, 0);
    op_create->created_edges = array_new(Edge*, 0);
    op_create->result_set = result_set;    

    _SetModifiedEntities(op_create);

    // Set our Op operations
    OpBase_Init(&op_create->op);
    op_create->op.name = "Create";
    op_create->op.type = OPType_CREATE;
    op_create->op.consume = OpCreateConsume;
    op_create->op.reset = OpCreateReset;
    op_create->op.free = OpCreateFree;

    return (OpBase*)op_create;
}

void _CreateNodes(OpCreate *op, Record r) {
    Graph *g = op->gc->g;
    for(int i = 0; i < op->node_count; i++) {
        /* Get specified node to create. */
        Node *n = op->nodes_to_create[i].node;

        /* Create a new node. */
        Node *newNode = Record_GetNode(r, op->nodes_to_create[i].node_rec_idx);
        newNode->entity = NULL;
        newNode->alias = n->alias;
        newNode->label = n->label;

        /* Save node for later insertion. */
        op->created_nodes = array_append(op->created_nodes, newNode);
    }
}

void _CreateEdges(OpCreate *op, Record r) {
    for(int i = 0; i < op->edge_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = Record_GetNode(r, op->edges_to_create[i].src_node_rec_idx);
        Node *dest_node = Record_GetNode(r, op->edges_to_create[i].dest_node_rec_idx);

        /* Create the actual edge. */
        Edge *newEdge = Record_GetEdge(r, op->edges_to_create[i].edge_rec_idx);
        newEdge->alias = e->alias;
        newEdge->relationship = e->relationship;
        Edge_SetSrcNode(newEdge, src_node);
        Edge_SetDestNode(newEdge, dest_node);

        /* Save edge for later insertion. */
        op->created_edges = array_append(op->created_edges, newEdge);
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op, TrieMap *createEntities) {
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
                op->result_set->stats.labels_added++;
            }
            labelID = schema->id;
        }

        // Introduce node into graph.
        Graph_CreateNode(g, labelID, n);

        // Set node properties.
        AST_GraphEntity *entity = TrieMap_Find(createEntities, n->alias, strlen(n->alias));
        assert(entity != NULL && entity != TRIEMAP_NOTFOUND);

        if(entity->properties) {
            int propCount = Vector_Size(entity->properties);
            if(propCount > 0) {
                for(int prop_idx = 0; prop_idx < propCount; prop_idx+=2) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(entity->properties, prop_idx, &key);
                    Vector_Get(entity->properties, prop_idx+1, &value);

                    Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)n, prop_id, *value);
                }
                // Introduce node to schema indices.
                if(n->label) GraphContext_AddNodeToIndices(op->gc, schema, n);
                op->result_set->stats.properties_set += propCount/2;
            }
        }
    }
    
    op->result_set->stats.nodes_created += node_count;
}

static void _CommitEdges(OpCreate *op, TrieMap *createEntities) {
    Edge *e;
    int labelID;
    Graph *g = op->gc->g;
    int relationships_created = 0;
    uint createdEdgeCount = array_len(op->created_edges);

    for(uint i = 0; i < createdEdgeCount; i++) {
        e = op->created_edges[i];
        int relation_id;
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
        relation_id = schema->id;

        if(!Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e)) continue;

        // Set edge properties.
        AST_GraphEntity *entity = TrieMap_Find(createEntities, e->alias, strlen(e->alias));
        assert(entity != NULL && entity != TRIEMAP_NOTFOUND);

        if(entity->properties) {
            int propCount = Vector_Size(entity->properties);
            if(propCount > 0) {
                for(int prop_idx = 0; prop_idx < propCount; prop_idx+=2) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(entity->properties, prop_idx, &key);
                    Vector_Get(entity->properties, prop_idx+1, &value);

                    Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)e, prop_id, *value);
                }
                op->result_set->stats.properties_set += propCount/2;
            }
        }
        relationships_created++;
    }
    
    op->result_set->stats.relationships_created += relationships_created;
}

static void _CommitNewEntities(OpCreate *op) {
    Graph *g = op->gc->g;
    TrieMap *createEntities = NewTrieMap();
    CreateClause_ReferredEntities(op->ast->createNode, createEntities);

    // Lock everything.
    Graph_AcquireWriteLock(g);
    Graph_SetMatrixPolicy(g, RESIZE_TO_CAPACITY);
    if(array_len(op->created_nodes) > 0) _CommitNodes(op, createEntities);
    if(array_len(op->created_edges) > 0) _CommitEdges(op, createEntities);
    Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);
    // Release lock.
    Graph_ReleaseLock(g);

    TrieMap_Free(createEntities, TrieMap_NOP_CB);
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
        AST *ast = op->ast;
        r = Record_New(AST_AliasCount(ast));
        /* Create entities. */
        _CreateNodes(op, r);
        _CreateEdges(op, r);

        // Save record for later use.
        op->records = array_append(op->records, r);
    } else {
        // Pull data until child is depleted.
        OpBase *child = op->op.children[0];
        while((r = OpBase_Consume(child))) {
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

    if(op->created_nodes) array_free(op->created_nodes);
    if(op->created_edges) array_free(op->created_edges);
    if(op->nodes_to_create) rm_free(op->nodes_to_create);
    if(op->edges_to_create) rm_free(op->edges_to_create);
}

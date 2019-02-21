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

void _SetModifiedEntities(OpCreate *op) {
    // TODO
    AST *ast = AST_GetFromLTS();
    /* Determin which entities are modified by create op. */
    size_t create_entity_count = Vector_Size(op->ast->createNode->graphEntities);
    op->nodes_to_create = malloc(sizeof(NodeCreateCtx) * create_entity_count);
    op->edges_to_create = malloc(sizeof(EdgeCreateCtx) * create_entity_count);

    int node_idx = 0;
    int edge_idx = 0;

    /* For every entity within the CREATE clause see if it's also mentioned 
     * within the MATCH clause. */
    TrieMap *matchEntities = NewTrieMap();
    MatchClause_DefinedEntities(op->ast->matchNode, matchEntities);

    for(int i = 0; i < create_entity_count; i++) {
        AST_GraphEntity *create_ge;
        Vector_Get(op->ast->createNode->graphEntities, i, &create_ge);
        char *alias = create_ge->alias;

        /* See if current entity needs to be created:
         * 1. current entity is NOT in MATCH clause.
         * 2. We've yet to accounted for this entity. */
        if(TrieMap_Add(matchEntities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE) == 0) continue;
        if(create_ge->t == N_ENTITY) {
            // Node.
            Node *n = QueryGraph_GetNodeByAlias(op->qg, create_ge->alias);
            Node **ppn = QueryGraph_GetNodeRef(op->qg, n);
            op->nodes_to_create[node_idx].node = n;
            op->nodes_to_create[node_idx].node_rec_idx = AST_GetAliasID(ast, n->alias);
            node_idx++;
        } else {
            // Edge.
            Edge *e = QueryGraph_GetEdgeByAlias(op->qg, create_ge->alias);            
            op->edges_to_create[edge_idx].edge = e;
            op->edges_to_create[edge_idx].edge_rec_idx = AST_GetAliasID(ast, e->alias);
            assert(QueryGraph_ContainsNode(op->qg, e->src));
            assert(QueryGraph_ContainsNode(op->qg, e->dest));
            op->edges_to_create[edge_idx].src_node_rec_idx = AST_GetAliasID(ast, e->src->alias);
            op->edges_to_create[edge_idx].dest_node_rec_idx = AST_GetAliasID(ast, e->dest->alias);
            edge_idx++;
        }        
    }

    TrieMap_Free(matchEntities, TrieMap_NOP_CB);

    op->node_count = node_idx;
    op->edge_count = edge_idx;
    /* Create must modify atleast one entity. */
    assert((op->node_count + op->edge_count) > 0);
}

OpBase* NewCreateOp(RedisModuleCtx *ctx, GraphContext *gc, AST *ast, QueryGraph *qg, ResultSet *result_set) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = gc;
    op_create->ast = ast;
    op_create->qg = qg;
    op_create->records = NULL;
    op_create->nodes_to_create = NULL;
    op_create->node_count = 0;
    op_create->edges_to_create = NULL;
    op_create->edge_count = 0;
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
        Node *newNode = Node_New(n->label, n->alias);

        /* Save node for later insertion. */
        op->created_nodes = array_append(op->created_nodes, newNode);

        /* Update record with new node. */
        Record_AddScalar(r, op->nodes_to_create[i].node_rec_idx, SI_PtrVal(newNode));
    }
}

void _CreateEdges(OpCreate *op, Record r) {
    for(int i = 0; i < op->edge_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].src_node_rec_idx);
        Node *dest_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].dest_node_rec_idx);

        /* Create the actual edge. */
        Edge *newEdge = Edge_New(src_node, dest_node, e->relationship, e->alias);

        /* Save edge for later insertion. */
        op->created_edges = array_append(op->created_edges, newEdge);

        /* Update record with new edge. */
        Record_AddScalar(r, op->edges_to_create[i].edge_rec_idx, SI_PtrVal(newEdge));
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op) {
    Node *n;
    int labelID;
    Graph *g = op->gc->g;
    uint node_count = array_len(op->created_nodes);
    TrieMap *createEntities = NewTrieMap();
    Schema *unified_schema = GraphContext_GetUnifiedSchema(op->gc, SCHEMA_NODE);
    
    Graph_AllocateNodes(op->gc->g, node_count);
    CreateClause_ReferredEntities(op->ast->createNode, createEntities);

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

                    Attribute_ID prop_id = Schema_AddAttribute(schema, SCHEMA_NODE, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)n, prop_id, *value);
                }
                // Introduce node to schema indices.
                if(n->label) GraphContext_AddNodeToIndices(op->gc, schema, n);
                op->result_set->stats.properties_set += propCount/2;
            }
        }
    }

    TrieMap_Free(createEntities, TrieMap_NOP_CB);
}

static void _CommitEdges(OpCreate *op) {
    Edge *e;
    int labelID;
    Graph *g = op->gc->g;
    int relationships_created = 0;
    TrieMap *createEntities = NewTrieMap();
    CreateClause_ReferredEntities(op->ast->createNode, createEntities);

    uint createdEdgeCount = array_len(op->created_edges);
    for(uint i = 0; i < createdEdgeCount; i++) {
        e = op->created_edges[i];
        int relation_id;
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

                    Attribute_ID prop_id = Schema_AddAttribute(schema, SCHEMA_EDGE, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)e, prop_id, *value);
                }
                op->result_set->stats.properties_set += propCount/2;
            }
        }
        relationships_created++;
    }

    TrieMap_Free(createEntities, TrieMap_NOP_CB);
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
    if(op->nodes_to_create) free(op->nodes_to_create);
    if(op->edges_to_create) free(op->edges_to_create);
    
    if (op->created_nodes) {
        size_t nodeCount = array_len(op->created_nodes);
        for(uint i = 0; i < nodeCount; i++) {
            // It's safe to free node, as its internal GraphEntity wouldn't be free.
            Node_Free(op->created_nodes[i]);
        }
        array_free(op->created_nodes);
    }

    if (op->created_edges) {
        size_t edgeCount = array_len(op->created_edges);
        for(uint i = 0; i < edgeCount; i++) {
            // It's safe to free edge, as its internal GraphEntity wouldn't be free.
            Edge_Free(op->created_edges[i]);
        }
        array_free(op->created_edges);
    }
}

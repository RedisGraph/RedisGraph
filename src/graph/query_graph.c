/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../util/arr.h"
#include "../schema/schema.h"
#include "../../deps/rax/rax.h"
#include <assert.h>

static void _QueryGraph_AddASTRef(QueryGraph *qg, const cypher_astnode_t *ast_entity, void *qg_entity) {
    int rc = TrieMap_Add(qg->ast_references, (char*)&ast_entity, sizeof(ast_entity), qg_entity, TrieMap_DONT_CARE_REPLACE);
    assert(rc == 1);
}

static void _QueryGraph_AddID(QueryGraph *qg, uint id, void *qg_entity) {
    int rc = TrieMap_Add(qg->ast_references, (char*)&id, sizeof(id), qg_entity, TrieMap_DONT_CARE_REPLACE);
    assert(rc == 1);
}

static void _BuildQueryGraphAddNode(const GraphContext *gc,
                             const AST *ast,
                             const cypher_astnode_t *ast_entity,
                             QueryGraph *qg) {

    // Look up this AST reference in the QueryGraph.
    QGNode *n = QueryGraph_GetEntityByASTRef(qg, ast_entity);

    // This AST node has already been added to the QueryGraph, do nothing.
    if (n) return;

    // Retrieve the AST ID of this reference.
    uint id = AST_GetEntityIDFromReference(ast, ast_entity);
    assert(id != IDENTIFIER_NOT_FOUND);

    // If this AST ID has already been mapped into the QueryGraph from a different AST entity,
    //  we'll augment the previously-built QGNode with any new AST data.
    // This call captures reused aliases.
    n = QueryGraph_GetEntityByASTID(qg, id);

    if (n == NULL) {
        // Node has not been mapped; create it.
        n = QGNode_New(NULL, NULL);

        const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(ast_entity);
        if (ast_alias) n->alias = cypher_ast_identifier_get_name(ast_alias);

        // Map the AST ID.
        _QueryGraph_AddID(qg, id, n);

        // TODO old dummy entity logic? What if we have multiple AST IDs?
        n->id = id;

        qg->nodes = array_append(qg->nodes, n);

    }

    // Add the current AST reference.
    _QueryGraph_AddASTRef(qg, ast_entity, (void*)n);

    // Retrieve node labels from the AST entity.
    uint nlabels = cypher_ast_node_pattern_nlabels(ast_entity);
    // We currently only support 0 or 1 labels per node, so if any are specified just select the first.
    const char *label = (nlabels > 0) ? cypher_ast_label_get_name(cypher_ast_node_pattern_get_label(ast_entity, 0)) : NULL;

    // Set node label ID if one has not already been set.
    if(n->labelID == GRAPH_NO_LABEL) {
        if (label) {
            Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
            uint label_id = GRAPH_UNKNOWN_LABEL;
            // If a schema is found, the AST refers to a real label.
            if(s) label_id = s->id;
            n->label = label;
            n->labelID = label_id;
        } else {
            n->labelID = GRAPH_NO_LABEL;
        }
    }

    // TODO properties?
}

static void _BuildQueryGraphAddEdge(const GraphContext *gc,
                        const AST *ast,
                        const cypher_astnode_t *ast_entity,
                        const cypher_astnode_t *l_entity,
                        const cypher_astnode_t *r_entity,
                        QueryGraph *qg) {

    // Each edge can only appear once in a QueryGraph.
    assert(QueryGraph_GetEntityByASTRef(qg, ast_entity) == NULL);;

    // Retrieve the AST ID of this reference.
    uint id = AST_GetEntityIDFromReference(ast, ast_entity);
    assert(id != IDENTIFIER_NOT_FOUND);

    // If this AST ID has already been mapped into the QueryGraph from a different AST entity,
    //  we'll augment the previously-built QGEdge with any new AST data.
    // This call captures reused aliases.
    assert(QueryGraph_GetEntityByASTID(qg, id) == NULL);

    QGEdge *e = QGEdge_New(NULL, NULL, NULL, NULL);
    e->id = id;

    const cypher_astnode_t *ast_alias = cypher_ast_rel_pattern_get_identifier(ast_entity);
    if (ast_alias) e->alias = cypher_ast_identifier_get_name(ast_alias);

    // Map the AST ID.
    _QueryGraph_AddID(qg, id, e);

    // TODO old dummy entity logic? What if we have multiple AST IDs?

    QGNode *src = QueryGraph_GetEntityByASTRef(qg, l_entity);
    QGNode *dest = QueryGraph_GetEntityByASTRef(qg, r_entity);

    // Swap the source and destination for left-pointing relations
    if(cypher_ast_rel_pattern_get_direction(ast_entity) == CYPHER_REL_INBOUND) {
        QGNode *tmp = src;
        src = dest;
        dest = tmp;
    }

    // Add the IDs of all reltype matrixes
    uint nreltypes = cypher_ast_rel_pattern_nreltypes(ast_entity);
    for (uint i = 0; i < nreltypes; i ++) {
        const char *reltype = cypher_ast_reltype_get_name(cypher_ast_rel_pattern_get_reltype(ast_entity, i));
        e->reltypes = array_append(e->reltypes, reltype);
        // TODO unsafe - we might not have a schema if we're creating in this query
        Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
        if(!s) {
            e->reltypeIDs = array_append(e->reltypeIDs, GRAPH_UNKNOWN_RELATION);
            continue; // TODO sensible?
        }
        e->reltypeIDs = array_append(e->reltypeIDs, s->id);
    }

    // Incase of a variable length edge, set edge min/max hops.
    const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(ast_entity);
    if (range) {
        const cypher_astnode_t *start = cypher_ast_range_get_start(range);
        if (start) e->minHops = AST_ParseIntegerNode(start);

        const cypher_astnode_t *end = cypher_ast_range_get_end(range);
        if (end) {
            e->maxHops = AST_ParseIntegerNode(end);
        } else {
            e->maxHops = EDGE_LENGTH_INF;
        }
    }

    // Add the current AST reference.
    _QueryGraph_AddASTRef(qg, ast_entity, (void*)e);

    QueryGraph_ConnectNodes(qg, src, dest, e);
}

QueryGraph* QueryGraph_New(uint node_cap, uint edge_cap) {
    QueryGraph *qg = rm_malloc(sizeof(QueryGraph));

    qg->nodes = array_new(QGNode*, node_cap);
    qg->edges = array_new(QGEdge*, edge_cap);

    qg->ast_references = NewTrieMap();

    return qg;
}

void QueryGraph_ConnectNodes(QueryGraph *qg, QGNode *src, QGNode *dest, QGEdge *e) {
    QGNode_ConnectNode(src, dest, e);
    e->src = src;
    e->dest = dest;
    qg->edges = array_append(qg->edges, e);
}

void QueryGraph_AddPath(const GraphContext *gc, const AST *ast, QueryGraph *qg, const cypher_astnode_t *path) {
    uint nelems = cypher_ast_pattern_path_nelements(path);
    /* Introduce nodes first. Nodes are positioned at every even offset
     * into the path (0, 2, ...) */
    for (uint i = 0; i < nelems; i += 2) {
        const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
        _BuildQueryGraphAddNode(gc, ast, ast_node, qg);
    }

    /* Every odd offset corresponds to an edge in a path. */
    for (uint i = 1; i < nelems; i += 2) {
        // TODO still need to introduce nodes first? not doing that would simplify this logic
        const cypher_astnode_t *l_entity = cypher_ast_pattern_path_get_element(path, i - 1);
        const cypher_astnode_t *r_entity = cypher_ast_pattern_path_get_element(path, i + 1);
        const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, i);

        _BuildQueryGraphAddEdge(gc, ast, entity, l_entity, r_entity, qg);
    }

}

/* Build a complete query graph from the clauses that can introduce entities
 * (MATCH, MERGE, and CREATE) */
// TODO Depending on how path uniqueness is specified, this may be too inclusive?
QueryGraph* BuildQueryGraph(const GraphContext *gc, const AST *ast) {
    uint node_count;
    uint edge_count;
    // The initial node and edge arrays will be large enough to accommodate all AST entities
    // (which is overkill, consider reducing)
    node_count = edge_count = ast->entity_map->cardinality;
    QueryGraph *qg = QueryGraph_New(node_count, edge_count);

    // We are interested in every path held in a MATCH or CREATE pattern,
    // and the (single) path described by a MERGE clause.

    const cypher_astnode_t **match_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_MATCH);
    uint match_count = array_len(match_clauses);
    for (uint i = 0; i < match_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match_clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }
    array_free(match_clauses);

    // MERGE clauses
    const cypher_astnode_t **merge_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_MERGE);
    uint merge_count = array_len(merge_clauses);
    for (uint i = 0; i < merge_count; i ++) {
        const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clauses[i]);
        QueryGraph_AddPath(gc, ast, qg, path);
    }
    array_free(merge_clauses);

    // TODO WITH entities?

    return qg;
}

void QueryGraph_AddCreateClauses(const GraphContext *gc, const AST *ast, QueryGraph *qg) {
    const cypher_astnode_t **create_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_CREATE);
    uint create_count = array_len(create_clauses);
    for (uint i = 0; i < create_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(create_clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }
    array_free(create_clauses);
}

void* QueryGraph_GetEntityByASTRef(const QueryGraph *qg, const cypher_astnode_t *ref) {
    void *ge = TrieMap_Find(qg->ast_references, (void*)&ref, sizeof(ref));
    if (ge == TRIEMAP_NOTFOUND) return NULL;
    return ge;
}

void* QueryGraph_GetEntityByASTID(const QueryGraph *qg, uint id) {
    void *ge = TrieMap_Find(qg->ast_references, (void*)&id, sizeof(id));
    if (ge == TRIEMAP_NOTFOUND) return NULL;
    return ge;
}

QGNode* QueryGraph_GetNodeByID(const QueryGraph *qg, uint id) {
    uint node_count = array_len(qg->nodes);
    for (uint i = 0; i < node_count; i ++) {
        if (id == qg->nodes[i]->id) return qg->nodes[i];
    }
    return NULL;
}

SchemaType QueryGraph_GetEntityTypeByAlias(const QueryGraph *qg, const char *alias) {
    // TODO weak logic
    uint count = array_len(qg->nodes);
    for (uint i = 0; i < count; i ++) {
        const char *entity_alias = qg->nodes[i]->alias;
        if (!entity_alias) continue;
        if (!strcmp(entity_alias, alias)) return SCHEMA_NODE;
    }

    // Unnecessary if the alias is guaranteed to be in QueryGraph.
    count = array_len(qg->edges);
    for (uint i = 0; i < count; i ++) {
        const char *entity_alias = qg->edges[i]->alias;
        if (!entity_alias) continue;
        if (!strcmp(entity_alias, alias)) return SCHEMA_EDGE;
    }

    return SCHEMA_UNKNOWN;
}

QueryGraph* QueryGraph_Clone(const QueryGraph *qg) {
    uint node_count = array_len(qg->nodes);
    uint edge_count = array_len(qg->edges);
    QueryGraph *clone = QueryGraph_New(node_count, edge_count);
    
    // Clone nodes.
    for(int i = 0; i < node_count; i++) {
        // Clones node without its edges.
        QGNode *n = QGNode_Clone(qg->nodes[i]);
        clone->nodes = array_append(clone->nodes, n);
    }

    // Clone edges.
    for(int i = 0; i < edge_count; i++) {
        QGEdge *e = qg->edges[i];
        // TODO is this lookup necessary? Can't we just pass e->src?
        QGNode *src = QueryGraph_GetNodeByID(clone, e->src->id);
        QGNode *dest = QueryGraph_GetNodeByID(clone, e->dest->id);
        QGEdge *clone_edge = QGEdge_Clone(e);
        QueryGraph_ConnectNodes(clone, src, dest, clone_edge);
    }

    return clone;
}

QGNode* QueryGraph_RemoveNode(QueryGraph *qg, QGNode *n) {
    assert(qg && n);

    // Make sure node exists.
    // if(!QueryGraph_ContainsNode(qg, n)) return NULL;

    /* Remove node from query graph.
     * Remove all edges associated with node. */
    uint incoming_edge_count = array_len(n->incoming_edges);
    uint outgoing_edge_count = array_len(n->outgoing_edges);

    for(uint i = 0; i < incoming_edge_count; i++) {
        QGEdge *e = n->incoming_edges[i];
        QueryGraph_RemoveEdge(qg, e);
    }
    for(uint i = 0; i < outgoing_edge_count; i++) {
        QGEdge *e = n->outgoing_edges[i];
        QueryGraph_RemoveEdge(qg, e);
    }

    // Remove node from graph nodes.
    uint node_count = array_len(qg->nodes);
    for(uint i = 0; i < node_count; i++) {
        if(n->id == qg->nodes[i]->id) {
            /* If this is not the last element,
             * remove it by migrating the last element
             * to the removed position. */
            QGNode *last = array_pop(qg->nodes);
            if (last != n) qg->nodes[i] = last;
            break;
        }
    }

    return n;
}

QGEdge* QueryGraph_RemoveEdge(QueryGraph *qg, QGEdge *e) {
    assert(qg && e);

    // Disconnect nodes connected by edge.
    QGNode_RemoveOutgoingEdge(e->src, e);
    QGNode_RemoveIncomingEdge(e->dest, e);

    /* Remove edge from query graph. */
    uint edge_count = array_len(qg->edges);
    for(uint i = 0; i < edge_count; i++) {
        if(e->id == qg->edges[i]->id) {
            /* If this is not the last element,
             * remove it by migrating the last element
             * to the removed position. */
            QGEdge *last = array_pop(qg->edges);
            if (last != e) qg->edges[i] = last;
            break;
        }
    }

    return e;
}

QueryGraph** QueryGraph_ConnectedComponents(const QueryGraph *qg) {
    QGNode *n;                              // Current node.
    QGNode **q = array_new(QGNode*, 1);     // Node frontier.
    void *seen;                             // Has node been visited?
    QueryGraph *g = QueryGraph_Clone(qg);   // Clone query graph.
    rax *visited;                           // Dictionary of visited nodes.
    QueryGraph **connected_components;      // List of connected components.

    // At least one connected component (the original graph).
    connected_components = array_new(QueryGraph*, 1);

    // As long as there are nodes to process.
    while(true) {
        visited = raxNew();

        // Get a random node and add it to the frontier.
        QGNode *s = g->nodes[0];
        q = array_append(q, s);

        // As long as there are nodes in the frontier.
        while(array_len(q) > 0) {
            n = array_pop(q);

            // Mark n as visited.
            if(!raxInsert(visited, (unsigned char*)&n->id, sizeof(n->id), NULL, NULL)) {
                // We've already processed n.
                continue;
            }

            // Expand node N by visiting all of its neighbors
            for(int i = 0; i < array_len(n->outgoing_edges); i++) {
                QGEdge *e = n->outgoing_edges[i];
                seen = raxFind(visited, (unsigned char*)&e->dest->id, sizeof(e->dest->id));
                if(seen == raxNotFound) q = array_append(q, e->dest);
            }
            for(int i = 0; i < array_len(n->incoming_edges); i++) {
                QGEdge *e = n->incoming_edges[i];
                seen = raxFind(visited, (unsigned char*)&e->src->id, sizeof(e->src->id));
                if(seen == raxNotFound) q = array_append(q, e->src);
            }
        }

        /* Visited comprise the connected component defined by S.
         * Remove all none reachable nodes from current connected component.
         * Remove connected component from graph. */
        QueryGraph *cc = QueryGraph_Clone(g);
        uint node_count = array_len(g->nodes);
        for(uint i = 0; i < node_count; i++) {
            void *reachable;
            n = g->nodes[i];
            reachable = raxFind(visited, (unsigned char*)&n->id, sizeof(n->id));

            /* If node is reachable, which means it is part of the 
             * connected component, then remove it from the graph,
             * otherwise, node isn't reachable, not part of the 
             * connected component. */
            if(reachable != raxNotFound) {
                QueryGraph_RemoveNode(g, n);
            } else {
                n = QueryGraph_GetNodeByID(cc, n->id);
                QueryGraph_RemoveNode(cc, n);
            }
        }
    
        connected_components = array_append(connected_components, cc);
        
        // Clear visited dict for next iteration.
        raxFree(visited);

        // Exit when graph is empty.
        if(array_len(g->nodes) == 0) break;
    }

    QueryGraph_Free(g);
    return connected_components;
}

void QueryGraph_Clear(QueryGraph *qg) {
    while(array_len(qg->edges) > 0) QueryGraph_RemoveEdge(qg, qg->edges[0]);
    while(array_len(qg->nodes) > 0) QueryGraph_RemoveNode(qg, qg->nodes[0]);
}

/* Frees entire graph. */
void QueryGraph_Free(QueryGraph* qg) {
    if (!qg) return;

    /* Free QueryGraph nodes. */
    uint nodeCount = array_len(qg->nodes);
    for (uint i = 0; i < nodeCount; i ++) {
        QGNode_Free(qg->nodes[i]);
    }

    /* Free QueryGraph edges. */
    uint edgeCount = array_len(qg->edges);
    for(uint i = 0; i < edgeCount; i ++) {
        QGEdge_Free(qg->edges[i]);
    }

    TrieMap_Free(qg->ast_references, TrieMap_NOP_CB);
    array_free(qg->nodes);
    array_free(qg->edges);
    rm_free(qg);
}

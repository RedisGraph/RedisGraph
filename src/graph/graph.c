#include "graph.h"
#include "graph_type.h"
#include "assert.h"

#define MAX(a, b) \
    ((a > b) ? a : b)

// Computes the number of blocks required to accommodate n nodes.
#define GRAPH_NODE_COUNT_TO_BLOCK_COUNT(n) \
    MAX(1, n / NODEBLOCK_CAP)

// Computes block index for given node id.
#define GRAPH_NODE_ID_TO_BLOCK_INDEX(id) \
    (id / NODEBLOCK_CAP)

#define GRAPH_ACTIVE_BLOCK(g) \
    g->nodes_blocks[GRAPH_NODE_ID_TO_BLOCK_INDEX(g->node_count)]

// Retrieves block in which node id resides.
#define GRAPH_GET_NODE_BLOCK(g, id) \
    g->nodes_blocks[GRAPH_NODE_ID_TO_BLOCK_INDEX(id)]

/*========================= Graph utility functions ========================= */
// Resize given matrix to match graph's adjacency matrix dimensions.
void _Graph_ResizeMatrix(const Graph *g, GrB_Matrix m) {
    GrB_Index n_rows;
    
    GrB_Matrix_nrows(&n_rows, m);
    if(n_rows != g->node_cap) {
        GrB_Info res = GxB_Matrix_resize(m, g->node_cap, g->node_cap);
        assert(res == GrB_SUCCESS);
    }
}

// Resize graph's node array to contain at least n nodes.
void _Graph_ResizeNodes(Graph *g, size_t n) {
    int total_nodes = g->node_count + n;

    // Make sure we have room to store nodes.
    if (total_nodes < g->node_cap)
        return;

    int last_block = g->block_count - 1;

    // Increase NodeBlock count by the smallest multiple required to contain all nodes
    int increase_factor = (total_nodes / g->node_cap) + 2;
    g->block_count *= increase_factor;

    g->nodes_blocks = realloc(g->nodes_blocks, sizeof(NodeBlock*) * g->block_count);
    // Create and link blocks.
    for(int i = last_block; i < g->block_count-1; i++) {
        NodeBlock *block = g->nodes_blocks[i];
        NodeBlock *next_block = NodeBlock_New();
        g->nodes_blocks[i+1] = next_block;
        block->next = next_block;
    }

    g->node_cap = g->block_count * NODEBLOCK_CAP;
    _Graph_ResizeMatrix(g, g->adjacency_matrix);
}

/*================================ Graph API ================================ */
Graph *Graph_New(size_t n) {
    assert(n > 0);
    Graph *g = malloc(sizeof(Graph));
    
    g->node_cap = GRAPH_NODE_COUNT_TO_BLOCK_COUNT(n) * NODEBLOCK_CAP;
    g->node_count = 0;
    g->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
    g->relation_count = 0;
    g->label_cap = GRAPH_DEFAULT_LABEL_CAP;
    g->label_count = 0;
    
    g->block_count = GRAPH_NODE_COUNT_TO_BLOCK_COUNT(n);
    g->nodes_blocks = malloc(sizeof(NodeBlock*) * g->block_count);
    
    // Allocates blocks.
    for(int i = 0; i < g->block_count; i++) {
        g->nodes_blocks[i] = NodeBlock_New();
        if(i > 0) {
            // Link blocks.
            g->nodes_blocks[i-1]->next = g->nodes_blocks[i];
        }
    }

    g->relations = malloc(sizeof(GrB_Matrix) * g->relation_cap);
    g->labels = malloc(sizeof(GrB_Matrix) * g->label_cap);
    GrB_Matrix_new(&g->adjacency_matrix, GrB_BOOL, g->node_cap, g->node_cap);
    return g;
}

Graph *Graph_Get(RedisModuleCtx *ctx, RedisModuleString *graph_name) {
    Graph *g = NULL;

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);

    // Key does not exists.
	if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) == GraphRedisModuleType) {
        g = RedisModule_ModuleTypeGetValue(key);
	}

    RedisModule_CloseKey(key);
    return g;
}

void Graph_CreateNodes(Graph* g, size_t n, int* labels, NodeIterator **it) {
    assert(g);

    _Graph_ResizeNodes(g, n);
    Node *node = NULL;
    NodeIterator *node_it = NodeIterator_New(GRAPH_ACTIVE_BLOCK(g),
                                        g->node_count,
                                        g->node_count + n,
                                        1);

    int idx = 0, node_id = g->node_count;
    if(labels) {
        while((node = NodeIterator_Next(node_it)) != NULL) {
            int l = labels[idx];
            if(l != GRAPH_NO_LABEL) {
                GrB_Matrix m = Graph_GetLabelMatrix(g, l);
                GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
            }
            idx++;
            node_id++;
        }
        // Reset iterator for caller.
        NodeIterator_Reset(node_it);
    }

    g->node_count += n;

    if(it != NULL) *it = node_it;
    else NodeIterator_Free(node_it);
}

void Graph_ConnectNodes(Graph *g, size_t n, GrB_Index *connections) {
    assert(g && connections);

    // Update graph's adjacency matrices, setting mat[dest,src] to 1.
    for(int i = 0; i < n; i+=3) {
        int src_id = connections[i];
        int dest_id = connections[i+1];
        int r = connections[i+2];

        GrB_Matrix_setElement_BOOL(g->adjacency_matrix, true, src_id, dest_id);

        if(r != GRAPH_NO_RELATION) {
            // Typed edge.
            GrB_Matrix m = Graph_GetRelationMatrix(g, r);
            GrB_Matrix_setElement_BOOL(m, true, src_id, dest_id);
        }
    }
}

Node* Graph_GetNode(const Graph *g, int id) {
    assert(g && id >= 0);

    int block_id = GRAPH_NODE_ID_TO_BLOCK_INDEX(id);

    // Make sure block_id is within range.
    if(block_id >= g->block_count) {
        return NULL;
    }

    NodeBlock *block = g->nodes_blocks[block_id];
    return &block->nodes[id%NODEBLOCK_CAP];
}

void Graph_LabelNodes(Graph *g, int start_node_id, int end_node_id, int label, NodeIterator **it) {
    assert(g &&
           start_node_id < g->node_count &&
           start_node_id >=0 &&
           start_node_id < end_node_id &&
           end_node_id < g->node_count);
    
    GrB_Matrix m = Graph_GetLabelMatrix(g, label);
    for(int node_id = start_node_id; node_id <= end_node_id; node_id++) {
        GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
    }
    
    if(it) {
        *it = NodeIterator_New(GRAPH_GET_NODE_BLOCK(g, start_node_id),
                               start_node_id,
                               end_node_id+1,
                               1);
    }
}

NodeIterator *Graph_ScanNodes(const Graph *g) {
    assert(g);
    return NodeIterator_New(g->nodes_blocks[0], 0, g->node_count, 1);
}

GrB_Matrix Graph_GetLabelMatrix(const Graph *g, int label_idx) {
    assert(g && label_idx < g->label_count);
    GrB_Matrix m = g->labels[label_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

int Graph_AddLabelMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new label matrix.
    if(g->label_count == g->label_cap) {
        g->label_cap += 4;   // allocate room for 4 new matrices.
        g->labels = realloc(g->labels, g->label_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->labels[g->label_count++], GrB_BOOL, g->node_cap, g->node_cap);
    return g->label_count-1;
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
    assert(g && relation_idx < g->relation_count);
    GrB_Matrix m = g->relations[relation_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

int Graph_AddRelationMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new relation matrix.
    if(g->relation_count == g->relation_cap) {
        g->relation_cap += 4;   // allocate room for 4 new matrices.
        g->relations = realloc(g->relations, g->relation_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->relations[g->relation_count++], GrB_BOOL, g->node_cap, g->node_cap);
    return g->relation_count-1;
}

void Graph_Free(Graph *g) {
    assert(g);
        
    // Free each node.
    // Node *node;
    // NodeIterator *it = NodeIterator_New(g->nodes_blocks[0], 0, g->node_count);
    // while((node = NodeIterator_Next(it)) != NULL) {
    //     Node_Free(node);
    // }
    // NodeIterator_Free(it);
    
    // Free node blocks.
    for(int i = 0; i<g->block_count; i++) {
        NodeBlock_Free(g->nodes_blocks[i]);
    }
    free(g->nodes_blocks);

    // Free matrices.
    GrB_Matrix_free(&g->adjacency_matrix);
    for(int i = 0; i < g->relation_count; i++) {
        GrB_Matrix m = g->relations[i];
        GrB_Matrix_free(&m);
    }
    free(g->relations);
    
    // Free matrices.
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix m = g->labels[i];
        GrB_Matrix_free(&m);
    }
    free(g->labels);

    free(g);
}

#include "graph.h"
#include "graph_type.h"
#include "assert.h"
#include "../arithmetic/tuples_iter.h"

#define MAX(a, b) \
    ((a > b) ? a : b)

// Computes the number of blocks required to accommodate n nodes.
#define GRAPH_NODE_COUNT_TO_BLOCK_COUNT(n) \
    MAX(1, n / NODEBLOCK_CAP)

// Computes block index for given node id.
#define GRAPH_NODE_ID_TO_BLOCK_INDEX(id) \
    (id / NODEBLOCK_CAP)

// Computes node position within a block.
#define GRAPH_NODE_POSITION_WITHIN_BLOCK(id) \
    (id % NODEBLOCK_CAP)

// Get currently active block.
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
    if(n_rows != g->node_count) {
        GrB_Info res = GxB_Matrix_resize(m, g->node_count, g->node_count);
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
}

/* Relocate src node to dest node, overriding dest. */
void _Graph_NodeBlockMigrateNode(Graph *g, int src, int dest) {
    // Get the block in which dest node resides.
    NodeBlock *destNodeBlock = GRAPH_GET_NODE_BLOCK(g, dest);
    
    // Get node position within its block.
    int destNodeBlockIdx = GRAPH_NODE_POSITION_WITHIN_BLOCK(dest);

    // Get the src node in the graph.
    Node *srcNode = Graph_GetNode(g, src);

    // Replace dest node with src node.
    destNodeBlock->nodes[destNodeBlockIdx] = *srcNode;
}

/* Relocate src row and column, overriding dest. */
void _Graph_MigrateRowCol(Graph *g, int src, int dest) {
    GrB_Vector row;
    GrB_Vector col;
    GrB_Vector zero;
    GrB_Descriptor desc;
    GrB_Index nrows = g->node_count;
    GrB_Matrix M = g->adjacency_matrix;

    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    // GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

    GrB_Vector_new(&row, GrB_BOOL, nrows);
    GrB_Vector_new(&col, GrB_BOOL, nrows);
    GrB_Vector_new(&zero, GrB_BOOL, nrows);

    // Clear dest column.
    GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, nrows, dest, NULL);

    // Migrate row.
    GrB_Col_extract(row, NULL, NULL, M, GrB_ALL, nrows, src, desc);
    GrB_Row_assign(M, NULL, NULL, row, dest, GrB_ALL, nrows, NULL);

    // Migrate column.
    GrB_Col_extract(col, NULL, NULL, M, GrB_ALL, nrows, src, NULL);
    GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest, NULL);

    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);

        // Clear dest column.
        GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, nrows, dest, NULL);

        // Migrate row.
        GrB_Col_extract(row, NULL, NULL, M, GrB_ALL, nrows, src, desc);
        GrB_Row_assign(M, NULL, NULL, row, dest, GrB_ALL, nrows, NULL);

        // Migrate column.
        GrB_Col_extract(col, NULL, NULL, M, GrB_ALL, nrows, src, NULL);
        GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest, NULL);
    }

    // Clean up
    GrB_Vector_free(&row);
    GrB_Vector_free(&col);
    GrB_Vector_free(&zero);
    GrB_Descriptor_free(&desc);
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

    g->_relations = malloc(sizeof(GrB_Matrix) * g->relation_cap);
    g->_labels = malloc(sizeof(GrB_Matrix) * g->label_cap);
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

    if(it != NULL) {
        *it = NodeIterator_New(GRAPH_ACTIVE_BLOCK(g),
                               g->node_count,
                               g->node_count + n,
                               1);
    }

    int node_id = g->node_count;
    g->node_count += n;

    _Graph_ResizeMatrix(g, g->adjacency_matrix);

    if(labels) {
        for(int idx = 0; idx < n; idx++) {
            int l = labels[idx];
            if(l != GRAPH_NO_LABEL) {
                GrB_Matrix m = Graph_GetLabelMatrix(g, l);
                GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
            }
            node_id++;
        }
    }
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
    assert(g && id >= 0 && id < g->node_count);

    int block_id = GRAPH_NODE_ID_TO_BLOCK_INDEX(id);

    // Make sure block_id is within range.
    if(block_id >= g->block_count) {
        return NULL;
    }

    NodeBlock *block = g->nodes_blocks[block_id];
    return &block->nodes[id%NODEBLOCK_CAP];
}

void Graph_DeleteNodes(Graph *g, int *IDs, size_t IDCount) {
    assert(g && IDs);
    if(IDCount == 0) return;

    typedef struct {
        int nodeID;         // Node being deleted.
        int replacementID;  // Node taking over.
        bool delete;        // No need to replace, simply delete.
    } Replacement;

    Replacement *replacements = malloc(sizeof(Replacement) * IDCount);

    /* Allocate replacement candidates. */
    for(int i = 0; i < IDCount; i++) {
        replacements[i].replacementID = g->node_count - (IDCount - i);
        replacements[i].delete = false;
    }

    /* Locate which soon to deleted nodes are also replacement candidates. */
    for(int i = 0; i < IDCount; i++) {
        int id = IDs[i];
        if(id >= (g->node_count - IDCount)) {
            int j = IDCount - (g->node_count - id);
            replacements[j].nodeID = id;
            replacements[j].delete = true;
        }
    }

    /* For nodes marked for deletion which do require a replacement
     * find a replacement which is not marked for quick deletion. */
    for(int j = 0, i = 0; i < IDCount; i++) {
        int id = IDs[i];
        // Require a replacement?
        if(id < (g->node_count - IDCount)) {
            // Locate a valid replacement.
            while(replacements[j].delete) j++;
            replacements[j++].nodeID = id;
        }
    }

    /* Replace removed nodes within node blocks. */
    for(int j = 0, i = 0; i < IDCount; i++) {
        Replacement r = replacements[i];
        // No need to perform replacement.
        if(r.delete) continue;
        // Override nodeID with replacementID.
        _Graph_NodeBlockMigrateNode(g, r.replacementID, r.nodeID);
    }

    /* Replace rows, columns. */
    for(int i = 0; i < IDCount; i++) {
        Replacement r = replacements[i];
        if(!r.delete) {
            _Graph_MigrateRowCol(g, r.replacementID, r.nodeID);
        }
    }

    // Zero vector to clear entire row/column.
    GrB_Vector zero;
    GrB_Vector_new(&zero, GrB_BOOL, g->node_cap);

    // Update label matrices.
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix M = Graph_GetLabelMatrix(g, i);
        for(int j = 0; j < IDCount; j++) {
            Replacement r = replacements[j];
            bool srcExists = false;
            bool destExists = false;
            GrB_Matrix_extractElement_BOOL(&srcExists, M, r.replacementID, r.replacementID);
            GrB_Matrix_extractElement_BOOL(&destExists, M, r.nodeID, r.nodeID);

            // Clear, dest.
            if(destExists) {
                if(!srcExists || (srcExists && r.delete) ) {
                    GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, g->node_cap, r.replacementID, NULL);
                }
            }

            // Set dest
            if(!destExists) {
                if(srcExists && !r.delete) {
                    GrB_Matrix_setElement_BOOL(M, true, r.nodeID, r.nodeID);
                }
            }
        }
    }

    g->node_count -= IDCount;

    // Force matrix resizing.
    _Graph_ResizeMatrix(g, g->adjacency_matrix);

    // Clean up.
    GrB_Vector_free(&zero);
    free(replacements);
}

void Graph_DeleteEdge(Graph *g, int src_id, int dest_id) {
    assert(src_id < g->node_count && dest_id < g->node_count);
    
    // See if there's an edge between src and dest.
    bool connected = false;
    GrB_Matrix_extractElement_BOOL(&connected, g->adjacency_matrix, src_id, dest_id);
    
    if(!connected) return;

    GrB_Matrix M = g->adjacency_matrix;
    GrB_Index nrows = g->node_count;

    GrB_Vector mask;
    GrB_Vector_new(&mask, GrB_BOOL, nrows);
    GrB_Vector_setElement_BOOL(mask, true, src_id);

    GrB_Vector col;
    GrB_Vector_new(&col, GrB_BOOL, nrows);

    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);
    GrB_Descriptor_set(desc, GrB_MASK, GrB_SCMP);

    // Extract column dest_id.
    GrB_Col_extract(col, mask, NULL, M, GrB_ALL, nrows, dest_id, desc);
    GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest_id, NULL);

    // Update relation matrices.
    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);
        connected = false;
        GrB_Matrix_extractElement_BOOL(&connected, M, src_id, dest_id);
        if(connected) {
            GrB_Col_extract(col, mask, NULL, M, GrB_ALL, nrows, dest_id, desc);
            GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest_id, NULL);
        }
    }

    GrB_Descriptor_free(&desc);
    GrB_Vector_free(&col);
}

void Graph_LabelNodes(Graph *g, int start_node_id, int end_node_id, int label, NodeIterator **it) {
    assert(g &&
           start_node_id < g->node_count &&
           start_node_id >= 0 &&
           start_node_id <= end_node_id &&
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
    GrB_Matrix m = g->_labels[label_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

int Graph_AddLabelMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new label matrix.
    if(g->label_count == g->label_cap) {
        g->label_cap += 4;   // allocate room for 4 new matrices.
        g->_labels = realloc(g->_labels, g->label_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->_labels[g->label_count++], GrB_BOOL, g->node_cap, g->node_cap);
    return g->label_count-1;
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
    assert(g && relation_idx < g->relation_count);
    GrB_Matrix m = g->_relations[relation_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

int Graph_AddRelationMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new relation matrix.
    if(g->relation_count == g->relation_cap) {
        g->relation_cap += 4;   // allocate room for 4 new matrices.
        g->_relations = realloc(g->_relations, g->relation_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->_relations[g->relation_count++], GrB_BOOL, g->node_cap, g->node_cap);
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
        GrB_Matrix m = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_free(&m);
    }
    free(g->_relations);
    
    // Free matrices.
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix m = Graph_GetLabelMatrix(g, i);
        GrB_Matrix_free(&m);
    }
    free(g->_labels);

    free(g);
}

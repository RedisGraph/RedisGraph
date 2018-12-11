/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <pthread.h>

#include "entities/node.h"
#include "entities/edge.h"
#include "../redismodule.h"
#include "../util/triemap/triemap.h"
#include "../util/datablock/datablock.h"
#include "../util/datablock/datablock_iterator.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

#define GRAPH_DEFAULT_NODE_CAP 16384         // Default number of nodes a graph can hold before resizing.
#define GRAPH_DEFAULT_EDGE_CAP 16384         // Default number of edges a graph can hold before resizing.
#define GRAPH_DEFAULT_RELATION_TYPE_CAP 16   // Default number of different relationship types a graph can hold before resizing.
#define GRAPH_DEFAULT_LABEL_CAP 16           // Default number of different labels a graph can hold before resizing.
#define GRAPH_NO_LABEL -1                    // Labels are numbered [0-N], -1 represents no label.
#define GRAPH_NO_RELATION -1                 // Relations are numbered [0-N], -1 represents no relation.

typedef enum {
    GRAPH_EDGE_DIR_INCOMING,
    GRAPH_EDGE_DIR_OUTGOING,
    GRAPH_EDGE_DIR_BOTH,
} GRAPH_EDGE_DIR;

typedef enum {
    SYNC_AND_MINIMIZE_SPACE,
    RESIZE_TO_CAPACITY,
    DISABLED,
} MATRIX_POLICY;

// Forward declaration of Graph struct
typedef struct Graph Graph;
// typedef for synchronization function pointer
typedef void (*SyncMatrixFunc)(const Graph*, GrB_Matrix);

struct Graph {
    DataBlock *nodes;                   // Graph nodes stored in blocks.
    DataBlock *edges;                   // Graph edges stored in blocks.
    GrB_Matrix adjacency_matrix;        // Adjacency matrix, holds all graph connections.
    GrB_Matrix *labels;                 // Label matrices.
    GrB_Matrix *relations;              // Relation matrices.
    GrB_Matrix *_relations_map;         // Maps from (relation, row, col) to edge id.
    pthread_mutex_t _mutex;             // Mutex for accessing critical sections.
    pthread_rwlock_t _rwlock;           // Read-write lock scoped to this specific graph
    bool _writelocked;                  // true if the read-write lock was acquired by a writer
    SyncMatrixFunc SynchronizeMatrix;   // Function pointer to matrix synchronization routine.
};
/* Graph synchronization functions
 * The graph is initialized with a read-write lock allowing
 * concurrent access from one writer or N readers. */
/* Acquire a lock that does not restrict access from additional reader threads */
void Graph_AcquireReadLock(Graph *g);

/* Acquire a lock for exclusive access to this graph's data */
void Graph_AcquireWriteLock(Graph *g);

/* Release the held lock */
void Graph_ReleaseLock(Graph *g);

/* Choose the current matrix synchronization policy. */
void Graph_SetMatrixPolicy(Graph *g, MATRIX_POLICY policy);

/* Synchronize and resize all matrices in graph. */
void Graph_ApplyAllPending(Graph *g);

// Create a new graph.
Graph *Graph_New (
    size_t node_cap,    // Allocation size for node datablocks and matrix dimensions.
    size_t edge_cap     // Allocation size for edge datablocks.
);

// Creates a new label matrix, returns id given to label.
int Graph_AddLabel (
    Graph *g
);

// Creates a new relation matrix, returns id given to relation.
int Graph_AddRelationType (
    Graph *g
);

// Make sure graph can hold an additional N nodes.
void Graph_AllocateNodes (
    Graph* g,               // Graph for which nodes will be added.
    size_t n                // Number of nodes to create.    
);

// Make sure graph can hold an additional N edges.
void Graph_AllocateEdges (
    Graph *g,               // Graph for which nodes will be added.
    size_t n                // Number of edges to create.
);

// Create a single node and labels it accordingly.
// Return newly created node.
void Graph_CreateNode (
    Graph* g,
    int label,
    Node* n
);

// Connects source node to destination node.
// Returns 1 if connection is formed, 0 otherwise.
int Graph_ConnectNodes (
    Graph *g,           // Graph on which to operate.
    NodeID src,         // Source node ID.
    NodeID dest,        // Destination node ID.
    int r,              // Edge type.
    Edge *e
);

// Removes node and all of its connections within the graph.
void Graph_DeleteNode (
    Graph *g,
    Node *node
);

// Removes an edge from Graph and updates graph relevent matrices.
int Graph_DeleteEdge (
    Graph *g,
    Edge *e
);

// All graph matrices are required to be squared NXN
// where N is Graph_RequiredMatrixDim.
size_t Graph_RequiredMatrixDim (
    const Graph *g
);

// Retrieves a node iterator which can be used to access
// every node in the graph.
DataBlockIterator *Graph_ScanNodes (
    const Graph *g
);

// Retrieves an edge iterator which can be used to access
// every edge in the graph.
DataBlockIterator *Graph_ScanEdges (
    const Graph *g
);

// Returns number of nodes in the graph.
size_t Graph_NodeCount (
    const Graph *g
);

// Returns number of edges in the graph.
size_t Graph_EdgeCount (
    const Graph *g
);

// Returns number of different edge types.
int Graph_RelationTypeCount (
    const Graph *g
);

// Returns number of different node types.
int Graph_LabelTypeCount (
    const Graph *g
);

// Retrieves node with given id from graph,
// Returns NULL if node wasn't found.
int Graph_GetNode (
    const Graph *g,
    NodeID id,
    Node *n
);

// Retrieves node label
// returns GRAPH_NO_LABEL if node has no label.
int Graph_GetNodeLabel (
    const Graph *g,
    NodeID nodeID
);

// Retrieves edge with given id from graph,
// Returns NULL if edge wasn't found.
int Graph_GetEdge (
    const Graph *g,
    EdgeID id,
    Edge *e
);

// Retrieves edge relation.
int Graph_GetEdgeRelation (
    const Graph *g,
    Edge *e
);

// Retrieves edges connecting source to destination,
// relation is optional, pass GRAPH_NO_RELATION if you do not care
// of edge type.
void Graph_GetEdgesConnectingNodes (
    const Graph *g,     // Graph to get edges from.
    NodeID srcID,       // Source node of edge
    NodeID destID,      // Destination node of edge
    int r,              // Edge type.
    Edge **edges        // array_t of edges connecting src to dest of type r.
);

// Get node edges.
void Graph_GetNodeEdges (
    const Graph *g,         // Graph to get edges from.
    const Node *n,          // Node to extract edges from.
    GRAPH_EDGE_DIR dir,     // Edge direction.
    int edgeType,           // Relation type.
    Edge **edges            // array_t incoming/outgoing edges.
);

// Retrieves the adjacency matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetAdjacencyMatrix (
    const Graph *g
);

// Retrieves a label matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetLabel (
    const Graph *g,     // Graph from which to get adjacency matrix.
    int label           // Label described by matrix.
);

// Retrieves a typed adjacency matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetRelationMatrix (
    const Graph *g,     // Graph from which to get adjacency matrix.
    int relation        // Relation described by matrix.
);

// Free graph.
void Graph_Free (
    Graph *g
);

#endif

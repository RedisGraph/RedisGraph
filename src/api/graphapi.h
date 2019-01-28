/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef REDISGRAPH_API_H
#define REDISGRAPH_API_H

// #ifdef DREDIS_MODULE_TARGET

// RG_VERSION: a single integer for comparing spec and version levels
#define RG_VERSION(major, minor, sub) \
    (((major)*1000ULL + (minor))*1000ULL + (sub))

// The version of this implementation, and the RedisGraph API version:
#define RG_DATE "Jan 28, 2019"
#define RG_MAJOR 1
#define RG_MINOR 0
#define RG_SUB   0

//------------------------------------------------------------------------------
// Include files required by RedisGraph
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>

//------------------------------------------------------------------------------
// The RedisGraph entities
//------------------------------------------------------------------------------
typedef uint64_t RG_Node ;
typedef uint64_t RG_Edge ;

//------------------------------------------------------------------------------
// the RedisGraph integer
//------------------------------------------------------------------------------
typedef uint64_t RG_Index ;

//------------------------------------------------------------------------------
// RedisGraph error and informational codes
//------------------------------------------------------------------------------

// All RedisGraph functions return a code that indicates if it was successful
// or not.

typedef enum
{
    RG_SUCCESS,             // all is well
    RG_NO_VALUE,            // value requested but not there
}
RG_Info ;

/* High level API Functions */
typedef struct RedisModuleCtx RedisModuleCtx;
typedef struct RedisModuleString RedisModuleString;
RG_Info RG_init                     // start up RedisGraphAPI
(
    RedisModuleCtx *ctx,            // redis module context
    RedisModuleString *graphid      // graph id
) ;

RG_Info RG_finalize ( ) ;           // finish RedisGraphAPI

//==============================================================================
//=== RedisGraph Node functionality ============================================
//==============================================================================

RG_Info RG_Node_new             // create a new node
(
    RG_Node *n,                 // handle of node to create
    const char* label           // label of node to create
) ;

RG_Info RG_Node_attributeCount  // get the number of attributes in a node
(
    RG_Index *n,                // node has n attributes
    RG_Node n                   // node to query
) ;

RG_Info RG_Node_getLabel        // get label attached to node
(
    char **label                // label extracted
    RG_Node n                   // handle of node to extract label from
);

//------------------------------------------------------------------------------
// RG_Node_setAttribute
//------------------------------------------------------------------------------

// Set a single scalar in a node.

RG_Info RG_Node_setAttribute_BOOL
(
    RG_Node n,              // handle of node to modify
    const char *name,       // name of attribute to set
    const bool value        // scalar to assign
) ;

RG_Info RG_Node_setAttribute_NUMERIC
(
    RG_Node n,              // handle of node to modify
    const char *name,       // name of attribute to set
    const double value      // scalar to assign
) ;

RG_Info RG_Node_setAttribute_STRING
(
    RG_Node n,              // handle of node to modify
    const char *name,       // name of attribute to set
    const char *value       // scalar to assign
) ;

//------------------------------------------------------------------------------
// RG_Node_getAttribute
//------------------------------------------------------------------------------

// Extract a single attribute from a node.
// Returns RG_SUCCESS if attribute is present, and sets x to its value.
// Returns RG_NO_VALUE if attribute is not present, and x is unmodified.

RG_Info RG_Node_getAttribute_BOOL
(    
    bool *value             // scalar extracted
    RG_Node n,              // handle of node to extract attribute from
    const char *name        // name of attribute to get
) ;

RG_Info RG_Node_getAttribute_NUMERIC
(    
    double *value           // scalar extracted
    RG_Node n,              // handle of node to extract attribute from
    const char *name        // name of attribute to get
) ;

RG_Info RG_Node_getAttribute_STRING
(    
    char **value            // scalar extracted
    RG_Node n,              // handle of node to extract attribute from
    const char *name        // name of attribute to get
) ;

//------------------------------------------------------------------------------
// RG_Node_traversal
//------------------------------------------------------------------------------

RG_Info RG_Node_neighborCount   // get the number of neighbors node is connected to
(
    RG_Index *n,                // node has n neighbors
    RG_Node n                   // node to query
);

// Returns RG_SUCCESS if neighbor(i) is present, and sets neighbor to its value.
// Returns RG_NO_VALUE if neighbor(i) is not present, and neighbor is unmodified.
RG_Info RG_Node_getNeighbor
(
    RG_Node *neighbor,          // neighbor extracted
    RG_Node n,                  // handle to node to extract neighbor from
    RG_Index i                  // neighbor index
);

//------------------------------------------------------------------------------
// RG_Node_delete
//------------------------------------------------------------------------------

RG_Info RG_Node_delete
(
    RG_Node *n              // handle of node to delete
) ;

//==============================================================================
//=== RedisGraph Edge functionality ============================================
//==============================================================================

RG_Info RG_Edge_new             // create a new edge
(
    RG_Edge *e,                 // handle of edge to create
    const char* label           // label of edge to create
    RG_Node src                 // source node
    RG_Node dest                // destination node
) ;

RG_Info RG_Edge_attributeCount  // get the number of attributes in a edge
(
    RG_Index *n,                // edge has n attributes
    RG_Edge e                   // edge to query
) ;

RG_Info RG_Edge_getLabel        // get label attached to edge
(
    char **label                // label extracted
    RG_Edge e                   // handle of edge to extract label from
);

RG_Info RG_Edge_getSrcNode      // get edge source node
(
    RG_Node *n,                 // node extracted
    RG_Edge e                   // handle of edge to extract source node from
);

RG_Info RG_Edge_getDestNode     // get edge destination node
(
    RG_Node *n,                 // node extracted
    RG_Edge e                   // handle of edge to extract destination node from
);

RG_Info RG_Edge_getConnectingEdge   // get edge connecting src to dest
(
    RG_Node src,                    // edge source node
    RG_Node dest,                   // edge destination node
    const char *label,              // edge label
    RG_Edge *e                      // handle to edge
);

//------------------------------------------------------------------------------
// RG_Edge_setAttribute
//------------------------------------------------------------------------------

// Set a single scalar in an edge.

RG_Info RG_Edge_setAttribute_BOOL
(
    RG_Edge e,              // handle of edge to modify
    const char *name,       // name of attribute to set
    const bool value        // scalar to assign
) ;

RG_Info RG_Edge_setAttribute_NUMERIC
(
    RG_Edge e,              // handle of edge to modify
    const char *name,       // name of attribute to set
    const double value      // scalar to assign
) ;

RG_Info RG_Edge_setAttribute_STRING
(
    RG_Edge e,              // handle of edge to modify
    const char *name,       // name of attribute to set
    const char *value       // scalar to assign
) ;

//------------------------------------------------------------------------------
// RG_Edge_getAttribute
//------------------------------------------------------------------------------

// Extract a single attribute from an edge.
// Returns RG_SUCCESS if attribute is present, and sets x to its value.
// Returns RG_NO_VALUE if attribute is not present, and x is unmodified.

RG_Info RG_Edge_getAttribute_BOOL
(    
    bool *value             // scalar extracted
    RG_Edge e,              // handle of edge to extract attribute from
    const char *name        // name of attribute to get
) ;

RG_Info RG_Edge_getAttribute_NUMERIC
(    
    double *value           // scalar extracted
    RG_Edge e,              // handle of edge to extract attribute from
    const char *name        // name of attribute to get
) ;

RG_Info RG_Edge_getAttribute_STRING
(    
    char **value            // scalar extracted
    RG_Edge e,              // handle of edge to extract attribute from
    const char *name        // name of attribute to get
) ;

//------------------------------------------------------------------------------
// RG_Edge_delete
//------------------------------------------------------------------------------

RG_Info RG_Edge_delete
(
    RG_Edge *n              // handle of edge to delete
) ;


//==============================================================================
//=== RedisGraph Traversal API =================================================
//==============================================================================

typedef struct RG_PropertyBag_opaque *RG_PropertyBag;

RG_Info RG_findNodes                    // returns all nodes having the label, and the wanted properties.
(
    const char *label,                  // node label
    const RG_PropertyBag *properties    // properties node must have
    RG_Index *nvals                     // number of nodes located
    RG_Node **nodes                     // nodes located
);

// Traversal descriptors

//------------------------------------------------------------------------------
// RG_TraversalDesc_out
//------------------------------------------------------------------------------

RG_Info RG_TraversalDesc_out    // Traverse from given node on outgoing edges.
(
    RG_Node n,                  // Node to traverse from
    const char *type,           // Type of out edges to follow
    RG_Node *o                  // reachable node(s)
);

RG_Info RG_TraversalDesc_outE   // Traverse from given node on outgoing edges.
(
    RG_Node n,                  // Node to traverse from
    const char *type,           // Type of out edges to follow
    RG_Edge *e                  // reachable edge(s)
);

RG_Info RG_TraversalDesc_outV   // Extract source node from edge
(
    RG_Edge e,                  // Edge to get src node from    
    RG_Node *n                  // Source node
);

//------------------------------------------------------------------------------
// RG_TraversalDesc_in
//------------------------------------------------------------------------------

RG_Info RG_TraversalDesc_in    // Traverse given node on incoming edges.
(
    RG_Node n,                  // Node to reach
    const char *type,           // Type of incoming edges to traverse
    RG_Node *i                  // reachable node(s)
);

RG_Info RG_TraversalDesc_inE   // Traverse given node on incoming edges.
(
    RG_Node n,                  // Node to reach
    const char *type,           // Type of incoming edges to traverse
    RG_Edge *e                  // reachable edge(s)
);

RG_Info RG_TraversalDesc_inV    // Extract destination node from edge.
(
    RG_Edge e,                  // Edge to get dest node from
    RG_Node *n                  // Destination node
);

//------------------------------------------------------------------------------
// RG_Has
//------------------------------------------------------------------------------

RG_Info RG_Has_BOOL             // Entity must have given attribute set to value
(
    RG_Entity e,                // Graph entity to add filter to
    const char *attribute,      // Attribute name
    bool v                      // Attribute value
);

RG_Info RG_Has_INT64
(
    RG_Entity e,                // Graph entity to add filter to
    const char *attribute,      // Attribute name
    int64_t v
);

RG_Info RG_Has_UINT64
(
    RG_Entity e,                // Graph entity to add filter to
    const char *attribute,      // Attribute name
    uint64_t v                  // Attribute value
);

RG_Info RG_Has_FP32
(
    RG_Entity e,                // Graph entity to add filter to
    const char *attribute,      // Attribute name
    float v                     // Attribute value
);

RG_Info RG_Has_FP64
(
    RG_Entity e,                // Graph entity to add filter to
    const char *attribute,      // Attribute name
    double v                    // Attribute value
);

#define RG_Has(e,a,v)                               \
    _Generic                                        \
    (                                               \
        (v),                                        \        
              bool      : RG_Has_BOOL   ,  \        
              int64_t   : RG_Has_INT64  ,  \
              uint64_t  : RG_Has_UINT64 ,  \        
              float     : RG_Has_FP32   ,  \        
              double    : RG_Has_FP64      \
    )                                               \
    (e, a, v)


//==============================================================================
//=== RedisGraph Low-level API =================================================
//==============================================================================

/* Incomplete structures for compiler checks but opaque access. */
typedef struct GraphContext GraphContext;
typedef struct GrB_Matrix GrB_Matrix;

/* Redis Graph own version of vector, matrix. */
typedef GrB_Vector RG_Vector;
typedef GrB_Matrix RG_Matrix;

/* Algorithm entry point. */
typedef int (*fpAlgorithmRun)(const GraphContext *gc, int argc, const char *argv[]);

RG_Matrix GraphContext_GetLabelMatrix(const GraphContext *gc, const char *label);
RG_Matrix GraphContext_GetRelationMatrix(const GraphContext *gc, const char *relation);
int Algorithms_Register(const char *name, fpAlgorithmRun fp);

// #endif  /* DREDIS_MODULE_TARGET */
#endif  /* REDISGRAPH_API_H */

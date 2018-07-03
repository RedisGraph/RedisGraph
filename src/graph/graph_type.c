#include "graph.h"
#include "graph_type.h"
#include "../arithmetic/tuples_iter.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphRedisModuleType;

void _GraphType_LoadMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
    /* Format:     
     *      #entries N
     *      (row index,column index) X N
     */

    GrB_Index i;
    GrB_Index j;
    GrB_Index nvals;
    nvals = RedisModule_LoadUnsigned(rdb);

    while(nvals) {
        i = RedisModule_LoadUnsigned(rdb);
        j = RedisModule_LoadUnsigned(rdb);
        GrB_Matrix_setElement_BOOL(m, true, i, j);
        nvals--;
    }
}

void _GraphType_LoadMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * adjacency matrix
     * #relation matrices
     *      #entries N
     *      (row index,column index) X N 
     * #label matrices
     *      #entries N
     *      (row index,column index) X N 
     */

    _GraphType_LoadMatrix(rdb, g->adjacency_matrix);

    uint64_t relationMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < relationMatricesCount; i++) {
        int matrixIdx = Graph_AddRelationMatrix(g);
        GrB_Matrix m = Graph_GetRelationMatrix(g, matrixIdx);
        _GraphType_LoadMatrix(rdb, m);
    }

    uint64_t labelMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < labelMatricesCount; i++) {
        int matrixIdx = Graph_AddLabelMatrix(g);
        GrB_Matrix m = Graph_GetLabelMatrix(g, matrixIdx);
        _GraphType_LoadMatrix(rdb, m);
    }
}

void _GraphType_LoadNodes(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #nodes
     *      #properties N
     *      (name, value type, value) X N
     */

    uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
    NodeIterator *iter;
    Graph_CreateNodes(g, nodeCount, NULL, &iter);

    Node *n;
    while((n = NodeIterator_Next(iter))) {
        uint64_t propCount = RedisModule_LoadUnsigned(rdb);
        size_t propNameLen;
        char *propName[propCount];
        SIValue propValue[propCount];

        for(int i = 0; i < propCount; i++) {
            propName[i] = RedisModule_LoadStringBuffer(rdb, &propNameLen);
            SIType t = RedisModule_LoadUnsigned(rdb);
            SIValue val;
            if(t & SI_NUMERIC) {
                propValue[i] = SI_DoubleVal(RedisModule_LoadDouble(rdb));
            } else {
                propValue[i] = SI_StringValC(RedisModule_LoadStringBuffer(rdb, NULL));
            }
        }

        Node_Add_Properties(n, propCount, propName, propValue);
    }

    NodeIterator_Free(iter);
}

void *GraphType_RdbLoad(RedisModuleIO *rdb, int encver) {
    /* Format:
     * #nodes
     *      #properties N
     *      (name, value type, value) X N 
     *
     * adjacency matrix
     * #relation matrices
     *      #entries N
     *      (row index,column index) X N 
     * #label matrices
     *      #entries N
     *      (row index,column index) X N 
     */
    
    if (encver > GRAPH_TYPE_ENCODING_VERSION) {
        return NULL;
    }

    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    // Load nodes.
    _GraphType_LoadNodes(rdb, g);

    // Load matrices.
    _GraphType_LoadMatrices(rdb, g);

    return g;
}

void _GraphType_SaveNode(RedisModuleIO *rdb, const Node *n) {
    /* Format:
     * #properties N
     * (name, value type, value) X N */

    RedisModule_SaveUnsigned(rdb, n->prop_count);

    for(int i = 0; i < n->prop_count; i++) {
        EntityProperty prop = n->properties[i];
        RedisModule_SaveStringBuffer(rdb, prop.name, strlen(prop.name));        
        
        RedisModule_SaveUnsigned(rdb, prop.value.type);
        
        if(prop.value.type & SI_NUMERIC) {
            RedisModule_SaveDouble(rdb, prop.value.doubleval);
        } else {
            RedisModule_SaveStringBuffer(rdb, prop.value.stringval.str, prop.value.stringval.len);
        }        
    }
}

void _GraphType_SaveNodes(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #nodes 
     * nodes */
    
    RedisModule_SaveUnsigned(rdb, g->node_count);
    NodeIterator *iter = Graph_ScanNodes(g);
    Node *n;
    while((n = NodeIterator_Next(iter))) {
        _GraphType_SaveNode(rdb, n);
    }

    NodeIterator_Free(iter);
}

void _GraphType_SaveMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
    /* Format:
     * #entries N
     * (row index,column index) X N */
    TuplesIter *iter = TuplesIter_new(m);
    
    GrB_Index nvals;
    GrB_Index row;
    GrB_Index col;
    
    GrB_Matrix_nvals(&nvals, m);
    RedisModule_SaveUnsigned(rdb, nvals);

    while(TuplesIter_next(iter, &row, &col) == TuplesIter_OK) {
        RedisModule_SaveUnsigned(rdb, row);
        RedisModule_SaveUnsigned(rdb, col);
    }

    TuplesIter_free(iter);
}

void _GraphType_SaveMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format: 
     * adjacency matrix
     * #relation matrices
     * relation matrices
     * #label matrices
     * label matrices
     * */
    
    _GraphType_SaveMatrix(rdb, g->adjacency_matrix);

    RedisModule_SaveUnsigned(rdb, g->relation_count);
    for(int i = 0; i < g->relation_count; i++) {
        GrB_Matrix m = Graph_GetRelationMatrix(g, i);
        _GraphType_SaveMatrix(rdb, m);
    }

    RedisModule_SaveUnsigned(rdb, g->label_count);
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix m = Graph_GetLabelMatrix(g, i);
        _GraphType_SaveMatrix(rdb, m);
    }
}

void GraphType_RdbSave(RedisModuleIO *rdb, void *value) {
    /* Format:
     * nodes 
     * relations */

    Graph *g = (Graph *)value;
    
    // Dump nodes.
    _GraphType_SaveNodes(rdb, g);
    
    // Dump relations.
    _GraphType_SaveMatrices(rdb, g);
}

void GraphType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphType_Free(void *value) {
  Graph *g = value;
  Graph_Free(g);
}

int GraphType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = GraphType_RdbLoad,
                               .rdb_save = GraphType_RdbSave,
                               .aof_rewrite = GraphType_AofRewrite,
                               .free = GraphType_Free};
  
  GraphRedisModuleType = RedisModule_CreateDataType(ctx, "graphtype", GRAPH_TYPE_ENCODING_VERSION, &tm);
  if (GraphRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}

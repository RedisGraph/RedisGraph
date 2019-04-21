/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_fulltext_create_index.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../redisearch_api.h"

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.createNodeIndex(index_name, label, attributes)
// CALL db.idx.fulltext.createNodeIndex('books', ['Book'], ['title', 'authors'])

typedef struct {
    GraphContext *gc;
    Schema *s;
} _fulltextIndexCtx;

static void _populateIndex (
    RSIndex *idx,
    Graph *g,
    int label_id,
    char **fields,
    uint *fields_ids,
    uint fields_count)
{
    Node node;
    NodeID node_id;
    GraphEntity *entity;
    EntityProperty *prop;
    GxB_MatrixTupleIter *it;
    const GrB_Matrix label_matrix = Graph_GetLabel(g, label_id);
    GxB_MatrixTupleIter_new(&it, label_matrix);

    while(true) {
        bool depleted = false;
        GxB_MatrixTupleIter_next(it, NULL, &node_id, &depleted);
        if(depleted) break;
        
        double score = 0;
        const char* lang = NULL;
        Graph_GetNode(g, node_id, &node);
        RSDoc *doc = RediSearch_CreateDocument(&node_id, sizeof(EntityID), score, lang);

        for(uint i = 0; i < fields_count; i++) {
            RedisModuleString* s;
            SIValue *v = GraphEntity_GetProperty((GraphEntity*)&node, fields_ids[i]);
            if(v == PROPERTY_NOTFOUND) continue;
            if(v->type != T_STRING) continue;
            RediSearch_DocumentAddFieldString(doc, fields[i], v->stringval, strlen(v->stringval), RSFLDTYPE_FULLTEXT);
        }

        RediSearch_SpecAddDocument(idx, doc);
    }

    GxB_MatrixTupleIter_free(it);
}

static int _getNodeAttribute(void* ctx, const char* fieldName, const void* id, char** strVal, double* doubleVal) {
    Node n;
    NodeID nId = *(NodeID*)id;
    _fulltextIndexCtx *indexCtx = ctx;
    GraphContext *gc = indexCtx->gc;
    Graph *g = gc->g;
    Schema *s = indexCtx->s;
    
    Graph_GetNode(g, nId, &n);
    Attribute_ID attrId = GraphContext_GetAttributeID(gc, fieldName);
    SIValue *v = GraphEntity_GetProperty((GraphEntity*)&n, attrId);
    int ret;
    if(v == PROPERTY_NOTFOUND) {
        ret = RSVALTYPE_NOTFOUND;
    } else if(v->type & T_STRING) {
        *strVal = v->stringval;
        ret = RSVALTYPE_STRING;
    } else if(v->type & SI_NUMERIC) {
        *doubleVal = SI_GET_NUMERIC(*v);
        ret = RSVALTYPE_DOUBLE;
    } else {
        // Skiping booleans.
        ret = RSVALTYPE_NOTFOUND;
    }
    return ret;
}

ProcedureResult fulltextCreateNodeIdxInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) < 2) return PROCEDURE_ERR;
    
    // Create full-text index.
    char *label = args[0];
    GraphContext *gc = GraphContext_GetFromTLS();
    _fulltextIndexCtx *indexCtx = rm_malloc(sizeof(_fulltextIndexCtx));
    Graph *g = gc->g;
    Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
    // Schema doesn't exists, TODO: report error.
    if(!s) return PROCEDURE_ERR;

    indexCtx->gc = gc;
    indexCtx->s = s;
    int label_id = s->id;
    uint fields_count = array_len(args) - 1;
    uint fields_ids[fields_count];
    char **fields = args+1; // Skip index name.

    // Validate that all specified fields exists.
    for(int i = 0; i < fields_count; i++) {
        char *field = fields[i];
        if(GraphContext_GetAttributeID(gc, field) == ATTRIBUTE_NOTFOUND) {
            // TODO: report error.
            return PROCEDURE_ERR;
        }
    }

    RSIndex *idx = RediSearch_CreateIndex(label, _getNodeAttribute, indexCtx);
    Schema_SetFullTextIndex(s, idx);
    for(int i = 0; i < fields_count; i++) {
        char *field = fields[i];
        fields_ids[i] = GraphContext_GetAttributeID(gc, field);
        // Introduce text fields.
        RediSearch_CreateTextField(idx, field);
    }

    // Introduce data.
    _populateIndex(idx, g, label_id, fields, fields_ids, fields_count);    
    return PROCEDURE_OK;
}

SIValue* fulltextCreateNodeIdxStep(ProcedureCtx *ctx) {
    return NULL;
}

ProcedureResult fulltextCreateNodeIdxFree(ProcedureCtx *ctx) {
    // Clean up.
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextCreateNodeIdxGen() {
    void *privateData = NULL;
    ProcedureOutput **output = array_new(ProcedureOutput*, 0);
    ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.createNodeIndex",
                                    PROCEDURE_VARIABLE_ARG_COUNT,
                                    output,
                                    fulltextCreateNodeIdxStep,
                                    fulltextCreateNodeIdxInvoke,
                                    fulltextCreateNodeIdxFree,
                                    privateData);

    return ctx;
}

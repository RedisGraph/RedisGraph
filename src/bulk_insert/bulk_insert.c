/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "bulk_insert.h"
#include "../stores/store.h"
#include <assert.h>

typedef struct {
    long long node_count;   // Number of nodes to create under this label.
    const char *label;      // Label.
    int label_id;           // ID given to label, recognized by graph object.
    int attribute_count;    // How may attributes are there for this label.
    char **attributes;      // Array of attributes.
} LabelDesc;

void _BulkInsert_Reply_With_Syntax_Error(RedisModuleCtx *ctx, const char* err) {
    RedisModule_ReplyWithError(ctx, err);
}

// Parse label from argv.
RedisModuleString** _BulkInsert_Parse_Label(RedisModuleCtx *ctx, LabelDesc *label,
                                             RedisModuleString **argv, int *argc) {
    // Minimum of 3 arguments: label name, number of labeled nodes and attribute count.
    if(*argc < 3) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, missing label parameters.");
        return NULL;
    }
    *argc -= 3;

    // Label's name.
    label->label = RedisModule_StringPtrLen(*argv++, NULL);

    // Number of nodes under this label.
    long long node_count = 0;
    if (RedisModule_StringToLongLong(*argv++, &node_count) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse labeled node count.");
        return NULL;
    }

    label->node_count = node_count;

    // Label's attribute count.
    long long attribute_count = 0;
    if(RedisModule_StringToLongLong(*argv++, &attribute_count) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse label attributes count.");
        return NULL;
    }
    label->attribute_count = attribute_count;

    if(attribute_count > 0) {
        if(*argc < attribute_count) {
            _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, missing label attributes.");
            return NULL;
        }
        *argc -= attribute_count;

        label->attributes = malloc(sizeof(char*) * attribute_count);
        for(int j = 0; j < attribute_count; j++) {
            char *attribute = (char*)RedisModule_StringPtrLen(*argv++, NULL);
            label->attributes[j] = attribute;   // Attribute is being duplicated by GraphEntity_Add_Properties.
        }
    }

    return argv;
}

RedisModuleString** _BulkInsert_Parse_Labels(RedisModuleCtx *ctx, GraphContext *gc, LabelDesc *labels, int label_count,
                                              RedisModuleString **argv, int *argc) {

    // Consume labels.
    for(int label_idx = 0; label_idx < label_count; label_idx++) {
        argv = _BulkInsert_Parse_Label(ctx, &labels[label_idx], argv, argc);
        if(argv == NULL) {
            // Free previous parsed labels.
            for(int i = 0; i < label_idx; i++) {
                if(labels[i].attribute_count > 0) {
                    free(labels[i].attributes);
                }
            }
            return NULL;
        }

        LabelStore *store = GraphContext_GetStore(gc, labels[label_idx].label, STORE_NODE);
        LabelStore *allStore = GraphContext_AllStore(gc, STORE_NODE);
        if(store == NULL) {
            store = GraphContext_AddLabel(gc, labels[label_idx].label);
        }

        labels[label_idx].label_id = store->id;
        LabelStore_UpdateSchema(store, labels[label_idx].attribute_count, labels[label_idx].attributes);
        LabelStore_UpdateSchema(allStore, labels[label_idx].attribute_count, labels[label_idx].attributes);
    }

    return argv;
}

RedisModuleString** _BulkInsert_Read_Labeled_Node_Attributes(RedisModuleCtx *ctx,
                                                              int attribute_count,
                                                              SIValue *values,
                                                              RedisModuleString **argv, int *argc) {

    if(*argc < attribute_count) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse labeled node attributes.");
        return NULL;
    }
    *argc -= attribute_count;

    for(int i = 0; i < attribute_count; i++) {
        char *attribute_val = (char*)RedisModule_StringPtrLen(*argv++, NULL);
        values[i] = SIValue_FromString(attribute_val);
    }

    return argv;
}

RedisModuleString** _BulkInsert_Read_Unlabeled_Node_Attributes(RedisModuleCtx *ctx,
                                                                char **keys,
                                                                SIValue *values,
                                                                long long attribute_count,
                                                                RedisModuleString **argv,
                                                                int *argc) {
    if(*argc < attribute_count * 2) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attributes.");
        return NULL;
    }
    *argc -= attribute_count * 2;

    // Read key value pairs.
    for(long long i = 0; i < attribute_count; i++) {
        keys[i] = (char*)RedisModule_StringPtrLen(*argv++, NULL);

        char *attribute_val = (char*)RedisModule_StringPtrLen(*argv++, NULL);
        values[i] = SIValue_FromString(attribute_val);
    }

    return argv;
}

RedisModuleString** _BulkInsert_Insert_Nodes(RedisModuleCtx *ctx, GraphContext *gc, size_t *nodes,
                                              RedisModuleString **argv, int *argc) {
    GraphEntity *n;                 // Current node.
    DataBlockIterator *it = NULL;   // Iterator over nodes.
    long long nodes_to_create = 0;  // Total number of nodes to create.

    Graph *g = gc->g;

    if(*argc < 1 || RedisModule_StringToLongLong(*argv++, &nodes_to_create) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of nodes.");
        return NULL;
    }
    *argc -= 1;

    *nodes = nodes_to_create;
    Graph_AllocateNodes(g, nodes_to_create);

    long long label_count = 0;          // Number of unique labels.
    int number_of_labeled_nodes = 0;    // Count number of nodes labeled so far.

    // Read number of unique labels.
    if(RedisModule_StringToLongLong(*argv++, &label_count) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of unique labels.");
        return NULL;
    }
    *argc -= 1;

    // This query handles labeled nodes.
    if(label_count > 0) {
        LabelDesc labels[label_count];
        argv = _BulkInsert_Parse_Labels(ctx, gc, labels, label_count, argv, argc);
        if(argv == NULL) return NULL;

        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            Node n;
            LabelDesc l = labels[label_idx];
            for(int i = 0; i < l.node_count; i++) {
                Graph_CreateNode(g, l.label_id, &n);
                if(l.attribute_count > 0) {
                    SIValue values[l.attribute_count];
                    // Set nodes attributes.
                    argv = _BulkInsert_Read_Labeled_Node_Attributes(ctx, l.attribute_count, values, argv, argc);
                    if(argv == NULL) break;
                    GraphEntity_Add_Properties((GraphEntity*)&n, l.attribute_count, l.attributes, values);
                }
            }
            number_of_labeled_nodes += l.node_count;
        }

        // Free label attributes.
        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            LabelDesc l = labels[label_idx];
            if (l.attribute_count > 0) free(l.attributes);
        }

        return argv;
    }
    // This query handles unlabeled nodes.

    // Retrieve the node store so that we can update the schema for
    // unlabeled nodes.
    LabelStore *allStore = GraphContext_AllStore(gc, STORE_NODE);

    // Unlabeled nodes.
    long long attribute_count = 0;
    long long unlabeled_node_count = nodes_to_create - number_of_labeled_nodes;
    for(int i = 0; i < unlabeled_node_count; i++) {
        Node n;
        Graph_CreateNode(g, GRAPH_NO_LABEL, &n);
        if(*argc < 1) {
            _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, expected additional unlabeled nodes.");
            return NULL;
        }

        // Total number of attributes for a single unlabeled node.
        if(RedisModule_StringToLongLong(*argv++, &attribute_count) != REDISMODULE_OK) {
            _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attribute count.");
            return NULL;
        }
        *argc -= 1;

        if (attribute_count == 0) continue;

        char* keys[attribute_count];
        SIValue values[attribute_count];

        argv = _BulkInsert_Read_Unlabeled_Node_Attributes(ctx, keys, values, attribute_count, argv, argc);
        if(argv == NULL) return NULL;
        GraphEntity_Add_Properties((GraphEntity*)&n, attribute_count, keys, values);
        LabelStore_UpdateSchema(allStore, attribute_count, keys);
    }

    return argv;
}

RedisModuleString** _BulkInsert_Insert_Edges(RedisModuleCtx *ctx, GraphContext *gc, size_t *edges,
                                              RedisModuleString **argv, int *argc) {
    typedef struct {
        long long edge_count;   // Total number of edges.
        const char *label;      // Label given to edges.
        int label_id;           // Internal label ID.
    } LabelRelation;

    long long relations_count = 0;  // Total number of edges to create.
    long long label_count = 0;      // Number of labels.

    if(*argc < 2) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse RELATION section.");
        return NULL;
    }
    *argc -= 2;

    // Read number of edges to create.
    if(RedisModule_StringToLongLong(*argv++, &relations_count) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of relations to create.");
        return NULL;
    }

    *edges = relations_count;
    Graph_AllocateEdges(gc->g, relations_count);

    // Read number of unique relation types.
    if(RedisModule_StringToLongLong(*argv++, &label_count) != REDISMODULE_OK) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of relation types.");
        return NULL;
    }

    long long total_edges = 0;
    LabelRelation labelRelations[label_count];

    if(label_count > 0) {
        if(*argc < label_count * 2) {
            _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse relation types.");
            return NULL;
        }
        *argc -= label_count * 2;

        long long edge_count;
        for(int i = 0; i < label_count; i++) {
            labelRelations[i].label = RedisModule_StringPtrLen(*argv++, NULL);
            if(RedisModule_StringToLongLong(*argv++, &labelRelations[i].edge_count) != REDISMODULE_OK) {
                char *err;
                asprintf(&err, "Bulk insert format error, failed to parse number of relations of type '%s'.", labelRelations[i].label);
                _BulkInsert_Reply_With_Syntax_Error(ctx, err);
                return NULL;
            }
            total_edges += labelRelations[i].edge_count;
            LabelStore *s = GraphContext_GetStore(gc, labelRelations[i].label, STORE_EDGE);
            if(!s) {
                s = GraphContext_AddRelationType(gc, labelRelations[i].label);
                // TODO: Add support for parsing properties on relations
            }
            labelRelations[i].label_id = s->id;
        }
    }

    if(*argc < relations_count * 2) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse relations.");
        return NULL;
    }
    *argc -= relations_count * 2;

    // Introduce relations.
    NodeID srcId;
    NodeID destId;
    for(int i = 0; i < label_count; i++) {
        LabelRelation labelRelation = labelRelations[i];

        for(int j = 0; j < labelRelation.edge_count; j++) {
            if(RedisModule_StringToLongLong(*argv++, (long long*)&srcId) != REDISMODULE_OK) {
                _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation source node id.");
                return NULL;
            }
            if(RedisModule_StringToLongLong(*argv++, (long long*)&destId) != REDISMODULE_OK) {
                _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation destination node id.");
                return NULL;
            }

            Edge e;
            Graph_ConnectNodes(gc->g, srcId, destId, labelRelation.label_id, &e);
        }
    }
    return argv;
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, size_t *nodes, size_t *edges, RedisModuleString **argv, int argc) {

    if(argc < 1) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse bulk insert sections.");
        return BULK_FAIL;
    }

    bool section_found = false;
    const char *section = RedisModule_StringPtrLen(*argv++, NULL);
    argc -= 1;

    //TODO: Keep track and validate argc, make sure we don't overflow.
    if(strcmp(section, "NODES") == 0) {
        section_found = true;
        argv = _BulkInsert_Insert_Nodes(ctx, gc, nodes, argv, &argc);
        if (argv == NULL) {
            return BULK_FAIL;
        } else if (argc == 0) {
            return BULK_OK;
        }
        section = RedisModule_StringPtrLen(*argv++, NULL);
        argc -= 1;
    }

    if(strcmp(section, "RELATIONS") == 0) {
        section_found = true;
        argv = _BulkInsert_Insert_Edges(ctx, gc, edges, argv, &argc);
        if (argv == NULL) {
            return BULK_FAIL;
        } else if (argc == 0) {
            return BULK_OK;
        }
        section = RedisModule_StringPtrLen(*argv++, NULL);
        argc -= 1;
    }

    // "END" can optionally be sent after completing all inserts to resize
    // matrices and flush pending operations, decreasing load time on immediately
    // subsequent query operations.
    if(strcmp(section, "END") == 0) {
        if (argc != 0) _BulkInsert_Reply_With_Syntax_Error(ctx, "Tokens found after 'END' directive.");
        // Set matrix sync/resize policies to default
        Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
        Graph_ApplyAllPending(gc->g);
        char *reply;
        int len = asprintf(&reply, "Successfully constructed %lu nodes and %lu edges in graph '%s'", Graph_NodeCount(gc->g), Graph_EdgeCount(gc->g), gc->graph_name);

        RedisModule_ReplyWithStringBuffer(ctx, reply, len);
        free(reply);
        return BULK_COMPLETE;
    }

    if (!section_found) {
        char *error;
        asprintf(&error, "Unexpected token %s, expected NODES or RELATIONS.", section);
        _BulkInsert_Reply_With_Syntax_Error(ctx, error);
        free(error);
        return BULK_FAIL;
    }

    if(argc > 0) {
        _BulkInsert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, extra arguments.");
        return BULK_FAIL;
    }

    return BULK_OK;
}


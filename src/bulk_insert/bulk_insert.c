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

void _Bulk_Insert_Reply_With_Syntax_Error(RedisModuleCtx *ctx, const char* err) {
    RedisModule_ReplyWithError(ctx, err);
}

// Parse label from argv.
RedisModuleString** _Bulk_Insert_Parse_Label(RedisModuleCtx *ctx, RedisModuleString **argv, int *argc, LabelDesc *label) {
    // Minimum of 3 arguments: label name, number of labeled nodes and attribute count.
    if(*argc < 3) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, missing label parameters.");
        return NULL;
    }
    *argc -= 3;

    // Label's name.
    label->label = RedisModule_StringPtrLen(*argv++, NULL);

    // Number of nodes under this label.
    long long node_count = 0;
    if (RedisModule_StringToLongLong(*argv++, &node_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse labeled node count.");
        return NULL;
    }

    label->node_count = node_count;
    
    // Label's attribute count.
    long long attribute_count = 0;
    if(RedisModule_StringToLongLong(*argv++, &attribute_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse label attributes count.");
        return NULL;
    }
    label->attribute_count = attribute_count;

    if(attribute_count > 0) {
        if(*argc < attribute_count) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, missing label attributes.");
            return NULL;
        }
        *argc -= attribute_count;

        label->attributes = malloc(sizeof(char*) * attribute_count);
        for(int j = 0; j < attribute_count; j++) {
            char *attribute = (char*)RedisModule_StringPtrLen(*argv++, NULL);
            label->attributes[j] = attribute;   // Attribute is being duplicated by Node_Add_Properties.
        }
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Parse_Labels(RedisModuleString **argv, int *argc, GraphContext *gc,
                                              LabelDesc *labels, size_t label_count) {
    RedisModuleCtx *ctx = gc->ctx;

    // Consume labels.
    for(int label_idx = 0; label_idx < label_count; label_idx++) {
        argv = _Bulk_Insert_Parse_Label(ctx, argv, argc, &labels[label_idx]);
        if(argv == NULL) {
            // Free previous parsed labels.
            for(int i = 0; i < label_idx; i++) {
                if(labels[i].attribute_count > 0) {
                    free(labels[i].attributes);
                }
            }
            return NULL;
        }

        // TODO: Have label store hold on to attributes array.
        LabelStore *store = GraphContext_GetNodeStore(gc, labels[label_idx].label);
        LabelStore *allStore = GraphContext_AllStore(gc, STORE_NODE);
        if(store == NULL) {
            int label_id = Graph_AddLabel(gc->g);
            store = GraphContext_AddNode(gc, labels[label_idx].label);
        }

        labels[label_idx].label_id = store->id;
        LabelStore_UpdateSchema(store, labels[label_idx].attribute_count, labels[label_idx].attributes);
        LabelStore_UpdateSchema(allStore, labels[label_idx].attribute_count, labels[label_idx].attributes);
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Read_Labeled_Node_Attributes(RedisModuleCtx *ctx,
                                                              RedisModuleString **argv,
                                                              int *argc, int attribute_count,
                                                              SIValue *values) {

    if(*argc < attribute_count) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse labeled node attributes.");
        return NULL;
    }
    *argc -= attribute_count;

    for(int i = 0; i < attribute_count; i++) {
        size_t attribute_len;
        char *attribute_val = (char*)RedisModule_StringPtrLen(*argv++, &attribute_len);
        values[i] = SIValue_FromString(attribute_val);
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Read_Unlabeled_Node_Attributes(RedisModuleCtx *ctx,
                                                                RedisModuleString **argv,
                                                                int *argc,
                                                                char **keys,
                                                                SIValue *values,
                                                                long long attribute_count) {
    if(*argc < attribute_count * 2) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attributes.");
        return NULL;
    }
    *argc -= attribute_count * 2;

    // Read key value pairs.
    for(long long i = 0; i < attribute_count; i++) {
        keys[i] = (char*)RedisModule_StringPtrLen(*argv++, NULL);

        size_t attribute_len;
        char *attribute_val = (char*)RedisModule_StringPtrLen(*argv++, &attribute_len);
        values[i] = SIValue_FromString(attribute_val);
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Insert_Nodes(RedisModuleString **argv, int *argc,
                                              GraphContext *gc, size_t *nodes) {
    GraphEntity *n;                 // Current node.
    DataBlockIterator *it = NULL;   // Iterator over nodes.
    long long nodes_to_create = 0;  // Total number of nodes to create.

    RedisModuleCtx *ctx = gc->ctx;
    Graph *g = gc->g;

    if(*argc < 1 || RedisModule_StringToLongLong(*argv++, &nodes_to_create) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of nodes.");
        return NULL;
    }
    *argc -= 1;

    *nodes = nodes_to_create;
    size_t start_offset = Graph_NodeCount(g);
    Graph_CreateNodes(g, nodes_to_create, NULL, &it);

    long long label_count = 0;          // Number of unique labels.
    int number_of_labeled_nodes = 0;    // Count number of nodes labeled so far.

    // Read number of unique labels.
    if(RedisModule_StringToLongLong(*argv++, &label_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of unique labels.");
        return NULL;
    }
    *argc -= 1;

    // Labeled nodes.
    if(label_count > 0) {
        LabelDesc labels[label_count];
        argv = _Bulk_Insert_Parse_Labels(argv, argc, gc, labels, label_count);
        if(argv == NULL) return NULL;
                
        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            LabelDesc l = labels[label_idx];

            // Label nodes.
            Graph_LabelNodes(g, number_of_labeled_nodes + start_offset, number_of_labeled_nodes + start_offset + l.node_count - 1,
                             l.label_id);

            if(l.attribute_count > 0) {
                SIValue values[l.attribute_count];
                // Set nodes attributes.
                for(int i = 0; i < l.node_count; i++) {
                    argv = _Bulk_Insert_Read_Labeled_Node_Attributes(ctx, argv, argc, l.attribute_count, values);
                    if(argv == NULL) break;
                    n = (GraphEntity*)DataBlockIterator_Next(it);
                    GraphEntity_Add_Properties(n, l.attribute_count, l.attributes, values);
                }
            } else {
                DataBlockIterator_Skip(it, l.node_count);
            }
            number_of_labeled_nodes += l.node_count;
        }

        // Free label attributes.
        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            LabelDesc l = labels[label_idx];
            if (l.attribute_count > 0) free(l.attributes);
        }

        if(argv == NULL) return NULL;
    }

    // Retrieve the node store so that we can update the schema for
    // unlabeled nodes.
    LabelStore *allStore = GraphContext_AllStore(gc, STORE_NODE);

    // Unlabeled nodes.
    long long attribute_count = 0;
    while((n = (GraphEntity*)DataBlockIterator_Next(it)) != NULL) {
        if(*argc < 1) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, expected additional unlabeled nodes.");
            return NULL;
        }

        // Total number of attributes for a single unlabeled node.
        if(RedisModule_StringToLongLong(*argv++, &attribute_count) != REDISMODULE_OK) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attribute count.");
            return NULL;
        }
        *argc -= 1;

        if (attribute_count == 0) continue;

        char* keys[attribute_count];
        SIValue values[attribute_count];

        argv = _Bulk_Insert_Read_Unlabeled_Node_Attributes(ctx, argv, argc, keys, values, attribute_count);
        if(argv == NULL) return NULL;
        GraphEntity_Add_Properties(n, attribute_count, keys, values);
        LabelStore_UpdateSchema(allStore, attribute_count, keys);
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Insert_Edges(RedisModuleString **argv, int *argc,
                                              GraphContext *gc, size_t *edges) {
    typedef struct {
        long long edge_count;   // Total number of edges.
        const char *label;      // Label given to edges.
        int label_id;           // Internal label ID.
    } LabelRelation;

    long long relations_count = 0;  // Total number of edges to create.
    long long label_count = 0;      // Number of labels.

    RedisModuleCtx *ctx = gc->ctx;
    Graph *g = gc->g;

    if(*argc < 2) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse RELATION section.");
        return NULL;
    }
    *argc -= 2;

    // Read number of edges to create.
    if(RedisModule_StringToLongLong(*argv++, &relations_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of relations to create.");
        return NULL;
    }

    *edges = relations_count;

    // Read number of unique labels.
    if(RedisModule_StringToLongLong(*argv++, &label_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of relation labels.");
        return NULL;
    }

    long long total_labeled_edges = 0;
    // As we can have over 500k relations, this is safer as a heap allocation.
    EdgeDesc *connections = malloc(relations_count * sizeof(EdgeDesc));
    LabelRelation labelRelations[label_count + 1];  // Extra one for unlabeled relations.

    if(label_count > 0) {
        if(*argc < label_count * 2) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse relations labels.");
            return NULL;
        }
        *argc -= label_count * 2;

        for(int i = 0; i < label_count; i++) {
            labelRelations[i].label = RedisModule_StringPtrLen(*argv++, NULL);
            if(RedisModule_StringToLongLong(*argv++, &labelRelations[i].edge_count) != REDISMODULE_OK) {
                _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse relation label edge count.");
                return NULL;
            }
            total_labeled_edges += labelRelations[i].edge_count;
            LabelStore *s = GraphContext_GetRelationStore(gc, labelRelations[i].label);
            if(!s) {
                Graph_AddRelation(g);
                s = GraphContext_AddRelation(gc, labelRelations[i].label);
                // TODO: once we'll support edge attribute, need to update store schema.
            }
            labelRelations[i].label_id = s->id;
        }
    }

    if(*argc < relations_count * 2) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse relations.");
        return NULL;
    }
    *argc -= relations_count * 2;

    // Update number of unlabeled relations.
    labelRelations[label_count].edge_count = relations_count - total_labeled_edges;
    labelRelations[label_count].label_id = GRAPH_NO_RELATION;

    // Introduce relations.
    int connection_idx = 0;
    for(int i = 0; i < label_count+1; i++) {
        LabelRelation labelRelation = labelRelations[i];

        for(int j = 0; j < labelRelation.edge_count; j++) {
            if(RedisModule_StringToLongLong(*argv++, (long long*)&connections[connection_idx].srcId) != REDISMODULE_OK) {
                _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation source node id.");
                return NULL;
            }
            if(RedisModule_StringToLongLong(*argv++, (long long*)&connections[connection_idx].destId) != REDISMODULE_OK) {
                _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation destination node id.");
                return NULL;
            }
            connections[connection_idx].relationId = labelRelation.label_id;
            connection_idx++;
        }
    }

    Graph_ConnectNodes(g, connections, relations_count, NULL);

    free(connections);

    return argv;
}

int Bulk_Insert(RedisModuleString **argv, int argc, GraphContext *gc,
                size_t *nodes, size_t *edges) {

    if(argc < 1) {
        _Bulk_Insert_Reply_With_Syntax_Error(gc->ctx, "Bulk insert format error, failed to parse bulk insert sections.");
        return BULK_FAIL;
    }

    bool section_found = false;
    const char *section = RedisModule_StringPtrLen(*argv++, NULL);
    argc -= 1;

    //TODO: Keep track and validate argc, make sure we don't overflow.
    if(strcmp(section, "NODES") == 0) {
        section_found = true;
        argv = _Bulk_Insert_Insert_Nodes(argv, &argc, gc, nodes);
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
        argv = _Bulk_Insert_Insert_Edges(argv, &argc, gc, edges);
    }

    if (!section_found) {
        char *error;
        asprintf(&error, "Unexpected token %s, expected NODES or RELATIONS.", section);
        _Bulk_Insert_Reply_With_Syntax_Error(gc->ctx, error);
        free(error);
        return BULK_FAIL;
    }

    if(argc != 0) {
        _Bulk_Insert_Reply_With_Syntax_Error(gc->ctx, "Bulk insert format error, extra arguments.");
        return BULK_FAIL;
    }
    return BULK_OK;
}


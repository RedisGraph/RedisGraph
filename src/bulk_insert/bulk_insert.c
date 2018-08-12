/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "bulk_insert.h"
#include "./stores/store.h"
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

RedisModuleString** _Bulk_Insert_Parse_Labels(RedisModuleCtx *ctx, RedisModuleString **argv,
                                              int *argc, Graph *g, const char* graph_name,
                                              LabelDesc *labels, size_t label_count) {

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
        LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graph_name, labels[label_idx].label);
        LabelStore *allStore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
        if(store == NULL) {
            int label_id = Graph_AddLabelMatrix(g);
            store = LabelStore_New(ctx, STORE_NODE, graph_name, labels[label_idx].label, label_id);
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

RedisModuleString** _Bulk_Insert_Insert_Nodes(RedisModuleCtx *ctx, RedisModuleString **argv,
                                              int *argc, Graph *g, const char *graph_name,
                                              size_t *nodes) {
    Node *n;                        // Current node.
    NodeIterator *it;               // Iterator over nodes.
    long long nodes_to_create = 0;  // Total number of nodes to create.

    if(*argc < 2) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attributes.");
        return NULL;
    }
    *argc -= 2;

    // Read number of nodes to create.
    if(RedisModule_StringToLongLong(*argv++, &nodes_to_create) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of nodes.");
        return NULL;
    }
    
    *nodes = nodes_to_create;
    int start_offset = Graph_NodeCount(g);
    Graph_CreateNodes(g, nodes_to_create, NULL, &it);

    long long label_count = 0;          // Number of unique lables.
    int number_of_labeled_nodes = 0;    // Count number of nodes been labeled so far.

    // Read number of unique labels.
    if(RedisModule_StringToLongLong(*argv++, &label_count) != REDISMODULE_OK) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse number of unique labels.");
        return NULL;
    }

    // Labeled nodes.
    if(label_count > 0) {
        LabelDesc labels[label_count];
        argv = _Bulk_Insert_Parse_Labels(ctx, argv, argc, g, graph_name, labels, label_count);
        if(argv == NULL) return NULL;
                
        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            LabelDesc l = labels[label_idx];

            // Label nodes.
            Graph_LabelNodes(g, number_of_labeled_nodes + start_offset, number_of_labeled_nodes + start_offset + l.node_count - 1,
                             l.label_id, NULL);

            if(l.attribute_count > 0) {
                SIValue values[l.attribute_count];
                // Set nodes attributes.
                for(int i = 0; i < l.node_count; i++) {
                    argv = _Bulk_Insert_Read_Labeled_Node_Attributes(ctx, argv, argc, l.attribute_count, values);
                    if(argv == NULL) break;
                    n = NodeIterator_Next(it);
                    Node_Add_Properties(n, l.attribute_count, l.attributes, values);
                }
            }
            number_of_labeled_nodes += l.node_count;
        }

        // Free label attributes.
        for(int label_idx = 0; label_idx < label_count; label_idx++) {
            LabelDesc l = labels[label_idx];
             if(l.attribute_count > 0) {
                 free(l.attributes);
             }
        }

        if(argv == NULL) return NULL;
    }

    // Unlabeled nodes.
    long long attribute_count = 0;
    while((n = NodeIterator_Next(it)) != NULL) {
        if(*argc < 1) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attributes.");
            return NULL;
        }
        *argc -= 1;

        // Total number of attribute for unlabeled node.
        if(RedisModule_StringToLongLong(*argv++, &attribute_count) != REDISMODULE_OK) {
            _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse unlabeled node attribute count.");
            return NULL;
        }

        char* keys[attribute_count];
        SIValue values[attribute_count];

        argv = _Bulk_Insert_Read_Unlabeled_Node_Attributes(ctx, argv, argc, keys, values, attribute_count);
        if(argv == NULL) return NULL;
        Node_Add_Properties(n, attribute_count, keys, values);
    }

    return argv;
}

RedisModuleString** _Bulk_Insert_Insert_Edges(RedisModuleCtx *ctx, RedisModuleString **argv,
                                              int *argc, Graph *g, const char *graph_name,
                                              size_t *edges) {
    typedef struct {
        long long edge_count;   // Total number of edges.
        const char *label;      // Label given to edges.
        int label_id;           // Internal label ID.
    } LabelRelation;

    long long relations_count = 0;  // Total number of edges to create.
    long long label_count = 0;      // Number of labels.

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
    GrB_Index *connections = malloc(relations_count * 3 * sizeof(GrB_Index));       // Triplet (src,dest,relation)
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
            LabelStore *s = LabelStore_Get(ctx, STORE_EDGE, graph_name, labelRelations[i].label);
            if(s != NULL) {
                labelRelations[i].label_id = s->id;
            } else {
                labelRelations[i].label_id = Graph_AddRelationMatrix(g);
                LabelStore *s = LabelStore_New(ctx, STORE_EDGE, graph_name,
                                               labelRelations[i].label,
                                               labelRelations[i].label_id);
                // TODO: once we'll support edge attribute, need to update store schema.
            }
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
            if(RedisModule_StringToLongLong(*argv++, (long long*)&connections[connection_idx++]) != REDISMODULE_OK) {
                _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation source node id.");
                return NULL;
            }
            if(RedisModule_StringToLongLong(*argv++, (long long*)&connections[connection_idx++]) != REDISMODULE_OK) {
                _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to read relation destination node id.");
                return NULL;
            }
            connections[connection_idx++] = labelRelation.label_id;
        }
    }

    Graph_ConnectNodes(g, relations_count*3, connections);

    free(connections);

    return argv;
}

void Bulk_Insert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, Graph *g,
                 const char *graph_name, size_t *nodes, size_t *edges) {

    if(argc < 1) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, failed to parse bulk insert sections.");
        return;
    }
    argc -= 1;

    const char *section = RedisModule_StringPtrLen(*argv++, NULL);

    //TODO: Keep track and validate argc, make sure we don't overflow.
    if(strcmp(section, "NODES") == 0) {
        argv = _Bulk_Insert_Insert_Nodes(ctx, argv, &argc, g, graph_name, nodes);
        if(argv == NULL || argc == 0) return;
        argc -= 1;
        section = RedisModule_StringPtrLen(*argv++, NULL);
     }

     if(strcmp(section, "RELATIONS") == 0) {
        argv = _Bulk_Insert_Insert_Edges(ctx, argv, &argc, g, graph_name, edges);
    }

    if(argc != 0) {
        _Bulk_Insert_Reply_With_Syntax_Error(ctx, "Bulk insert format error, extra arguments.");
    }
}

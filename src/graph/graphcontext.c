#include <sys/param.h>
#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../redismodule.h"

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

GraphContext* GraphContext_New(RedisModuleCtx *ctx, const char *graphname,
                               size_t node_cap, size_t edge_cap) {
  GraphContext *gc = NULL;

  // Create key for GraphContext from the unmodified string provided by the user
  RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    goto cleanup;
  }

  gc = rm_malloc(sizeof(GraphContext));

  // No indicies.
  gc->index_count = 0;

  // Initialize the graph's matrices and datablock storage
  gc->g = Graph_New(node_cap, edge_cap);
  gc->graph_name = rm_strdup(graphname);
  // Allocate the default space for schemas and indices
  gc->node_schemas = array_new(Schema*, GRAPH_DEFAULT_LABEL_CAP);
  gc->relation_schemas = array_new(Schema*, GRAPH_DEFAULT_RELATION_TYPE_CAP);

  // Initialize the generic schemas
  gc->node_unified_schema = Schema_New("ALL", GRAPH_NO_LABEL);
  gc->relation_unified_schema = Schema_New("ALL", GRAPH_NO_RELATION);

  pthread_setspecific(_tlsGCKey, gc);

  // Set and close GraphContext key in Redis keyspace
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);

cleanup:
  RedisModule_CloseKey(key);
  RedisModule_FreeString(ctx, rs_name);
  return gc;
}

GraphContext* GraphContext_Retrieve(RedisModuleCtx *ctx, const char *graphname) {
  GraphContext *gc = NULL;
  RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_READ);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
    goto cleanup;
  }
  gc = RedisModule_ModuleTypeGetValue(key);

  // Force GraphBLAS updates and resize matrices to node count by default
  Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

  pthread_setspecific(_tlsGCKey, gc);

cleanup:
  RedisModule_FreeString(ctx, rs_name);
  RedisModule_CloseKey(key);
  return gc;
}

GraphContext* GraphContext_GetFromTLS() {
  GraphContext* gc = pthread_getspecific(_tlsGCKey);
  assert(gc);
  return gc;
}

//------------------------------------------------------------------------------
// Schema API
//------------------------------------------------------------------------------
// Find the ID associated with a label for schema and matrix access
int _GraphContext_GetLabelID(const GraphContext *gc, const char *label, SchemaType t) {
  // Choose the appropriate schema array given the entity type
  Schema **schemas = (t == SCHEMA_NODE) ? gc->node_schemas : gc->relation_schemas;

  // TODO optimize lookup
  for (uint32_t i = 0; i < array_len(schemas); i ++) {
    if (!strcmp(label, schemas[i]->name)) return i;
  }
  return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

unsigned short GraphContext_SchemaCount(const GraphContext *gc, SchemaType t) {
  assert(gc);
  if(t == SCHEMA_NODE) return array_len(gc->node_schemas);
  else return array_len(gc->relation_schemas);
}

Schema* GraphContext_GetUnifiedSchema(const GraphContext *gc, SchemaType t) {
  if (t == SCHEMA_NODE) return gc->node_unified_schema;
  return gc->relation_unified_schema;
}

Schema* GraphContext_GetSchemaByID(const GraphContext *gc, int id, SchemaType t) {
  Schema **schemas = (t == SCHEMA_NODE) ? gc->node_schemas : gc->relation_schemas;
  if (id == GRAPH_NO_LABEL) return NULL;
  return schemas[id];
}

Schema* GraphContext_GetSchema(const GraphContext *gc, const char *label, SchemaType t) {
  int id = _GraphContext_GetLabelID(gc, label, t);
  return GraphContext_GetSchemaByID(gc, id, t);
}

Schema* GraphContext_AddSchema(GraphContext *gc, const char *label, SchemaType t) {
  int label_id;
  Schema *schema;

  if(t == SCHEMA_NODE) {
    label_id = Graph_AddLabel(gc->g);
    schema = Schema_New(label, label_id);
    gc->node_schemas = array_append(gc->node_schemas, schema);
  } else {
    label_id = Graph_AddRelationType(gc->g);
    schema = Schema_New(label, label_id);
    gc->relation_schemas = array_append(gc->relation_schemas, schema);
  }

  return schema;
}

const char* GraphContext_GetNodeLabel(const GraphContext *gc, Node *n) {
    int label_id = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(n));
    if (label_id == GRAPH_NO_LABEL) return NULL;
    return gc->node_schemas[label_id]->name;
}

const char* GraphContext_GetEdgeRelationType(const GraphContext *gc, Edge *e) {
    int reltype_id = Graph_GetEdgeRelation(gc->g, e);
    if (reltype_id == GRAPH_NO_RELATION) return NULL;
    return gc->relation_schemas[reltype_id]->name;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
  return (gc->index_count > 0);
}

Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *attribute) {
  // Retrieve the schema for this label
  Schema *schema = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
  if (schema == NULL) return NULL;

  Index *idx = Schema_GetIndex(schema, attribute);
  return idx;
}

int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *attribute) {
  // Retrieve the schema for this label
  Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
  if (s == NULL) return INDEX_FAIL;

  // Verify that attribute exists and is not already indexed.
  Index *idx = Schema_GetIndex(s, attribute);
  if(idx) return INDEX_FAIL;

  // Populate an index for the label-attribute pair using the Graph interfaces.
  Attribute_ID attr_id = Schema_GetAttributeID(s, attribute);
  // Return if attribute does not exist.
  if (attr_id == ATTRIBUTE_NOTFOUND) return INDEX_FAIL;

  idx = Index_Create(gc->g, label, s->id, attribute, attr_id);

  // Associate the new index with the attribute in the schema.
  Schema_AddIndex(s, (char*)attribute, idx);

  gc->index_count++;
  return INDEX_OK;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *attribute) {
  // Retrieve the schema for this label
  Schema *schema = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
  if (schema == NULL) return INDEX_FAIL;

  Index *idx = Schema_GetIndex(schema, attribute);
  // Property does not exist or was not indexed.
  if(!idx) return INDEX_FAIL;

  // Remove the index association from the label schema
  Schema_RemoveIndex(schema, (char*)attribute);

  gc->index_count--;
  return INDEX_OK;
}

// Add references to a node to all indices built upon its properties
void GraphContext_AddNodeToIndices(GraphContext *gc, Schema *s, Node *n) {
  if(!s || !GraphContext_HasIndices(gc)) return;

  /* For each index in schema, see if node 
   * contains indexed attribute. */
  unsigned int index_count = Schema_IndexCount(s);
  for(unsigned int i = 0; i < index_count; i++) {
    Index *idx = s->indices[i];
    Attribute_ID attr_id = Schema_GetAttributeID(s, idx->attribute);
    // See if node contains current property.
    SIValue *v = GraphEntity_GetProperty((GraphEntity*)n, attr_id);
    if(v == PROPERTY_NOTFOUND) continue;

    Index_InsertNode(idx, n->entity->id, v);
  }
}

// Delete all references to a node from any indices built upon its properties
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, Node *n) {
  if (!GraphContext_HasIndices(gc)) return;

  Schema *s = NULL;
  EntityID node_id = ENTITY_GET_ID(n);
  if (n->label) {
    // Node will have a label string if one was specified in the query MATCH clause
    s = GraphContext_GetSchema(gc, n->label, SCHEMA_NODE);
  } else {
    // Otherwise, look up the offset of the matching label (if any)
    int schema_id = Graph_GetNodeLabel(gc->g, node_id);
    // Do nothing if node had no label
    if (schema_id == GRAPH_NO_LABEL) return;
    s = GraphContext_GetSchemaByID(gc, schema_id, SCHEMA_NODE);
  }

  // Update any indices this entity is represented in
  unsigned short idx_count = Schema_IndexCount(s);
  for(unsigned short i = 0; i < idx_count; i++) {
    Index *idx = s->indices[i];
    // See if node contains current property.
    SIValue *v = GraphEntity_GetProperty((GraphEntity*)n, idx->attr_id);
    if(v == PROPERTY_NOTFOUND) continue;
    Index_DeleteNode(idx, node_id, v);
  }
}

//------------------------------------------------------------------------------
// Free routine
//------------------------------------------------------------------------------

// Free all data associated with graph
void GraphContext_Free(GraphContext *gc) {
  Graph_Free(gc->g);
  rm_free(gc->graph_name);

  // Free generic schemas
  Schema_Free(gc->node_unified_schema);
  Schema_Free(gc->relation_unified_schema);

  // Free all node schemas
  if(gc->node_schemas) {
    for (uint32_t i = 0; i < array_len(gc->node_schemas); i ++) {
      Schema_Free(gc->node_schemas[i]);
    }
    array_free(gc->node_schemas);
  }

  // Free all relation schemas
  if(gc->relation_schemas) {
    for (uint32_t i = 0; i < array_len(gc->relation_schemas); i ++) {
      Schema_Free(gc->relation_schemas[i]);
    }
    array_free(gc->relation_schemas);
  }

  rm_free(gc);
}

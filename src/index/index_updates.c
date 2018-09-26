#include "index_updates.h"

void _index_DeleteNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistDelete(sl, val, &node);
}

void _index_InsertNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistInsert(sl, val, node);
}

void _index_UpdateNodeID(Index *idx, NodeID prev_id, NodeID new_id, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistNode *node = skiplistFind(sl, val);
  assert(node);
  int i;
  for (i = 0; i < node->numVals; i ++) {
    if (node->vals[i] == prev_id) {
      node->vals[i] = new_id;
      break;
    }
  }
  // Ensure that we found the node.
  assert(i < node->numVals);
}

Index* _index_retrieve(const char *label, const char *property) {
  for (int i = 0; i < gc->index_ctr; i ++) {
    Index *idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) {
      return idx;
    }
  }
  return NULL;
}

void Index_UpdateNodeValue(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID id, EntityProperty *oldval, SIValue *newval) {
  // Retrieve and iterate over all labels associated with node
  uint32_t node_labels[g->label_count];
  uint32_t label_count = Graph_GetNodeLabels(g, id, node_labels);
  for (uint32_t i = 0; i < label_count; i ++) {
    uint32_t label_id = node_labels[i];
    const char *label = Graph_GetLabelName(g, label_id);

    Index *idx = _index_retrieve(label, oldval->name);
    if (!idx) continue;

  /* Replace old property with new.
   * It is valid for the key type to have changed in the property update,
   * so the type-checking logic in these calls is not redundant. */
  _index_DeleteNode(idx, id, &oldval->value);
  _index_InsertNode(idx, id, newval);
  }
}


/* Invoked from the op_delete context */
void Indices_DeleteNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, size_t IDCount) {
  if (!GraphContext_HasIndices()) return;

  for (int i = 0; i < IDCount; i ++) {
    NodeID current_node = IDs[i];
    Node *delete_candidate = Graph_GetNode(g, current_node);
    // Find all matching labels for each node
    uint32_t node_labels[g->label_count];
    uint32_t label_count = Graph_GetNodeLabels(g, current_node, node_labels);
    for (int j = 0; j < label_count; j ++) {
      uint32_t label_id = node_labels[j];
      const char *label = Graph_GetLabelName(g, label_id);

      for (int k = 0; k < delete_candidate->prop_count; k ++) {
        const char *property = delete_candidate->properties[k].name;
        Index *idx = _index_retrieve(label, property);
        if (idx) _index_DeleteNode(idx, current_node, &delete_candidate->properties[k].value);
      }
    }
  }
}

/* Invoked from the op_delete context */
void Indices_UpdateNodeIDs(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, NodeID *replacementIDs, size_t IDCount) {
  if (!GraphContext_HasIndices()) return;

  for (int i = 0; i < IDCount; i ++) {
    NodeID current_node = IDs[i];
    Node *migration_candidate = Graph_GetNode(g, current_node);
    // Find all matching labels for each node
    uint32_t node_labels[g->label_count];
    uint32_t label_count = Graph_GetNodeLabels(g, current_node, node_labels);
    for (int j = 0; j < label_count; j ++) {
      uint32_t label_id = node_labels[j];
      const char *label = Graph_GetLabelName(g, label_id);

      for (int k = 0; k < migration_candidate->prop_count; k ++) {
        const char *property = migration_candidate->properties[k].name;
        Index *idx = _index_retrieve(label, property);
        if (idx) _index_UpdateNodeID(idx, current_node, replacementIDs[i], &migration_candidate->properties[k].value);
      }
    }
  }
}

/* Invoked from the op_create context */
void Indices_AddNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, int *labels, NodeID start_id, NodeID end_id) {
  if (!GraphContext_HasIndices()) return;

  for (int id = start_id; id < end_id; id ++) {
    Node *current = Graph_GetNode(g, id);
    int label_id = labels[id - start_id];
    if (label_id == GRAPH_NO_LABEL) continue;
    const char *label = Graph_GetLabelName(g, label_id);
    for (int j = 0; j < current->prop_count; j ++) {
      const char *property = current->properties[j].name;
      Index *idx = _index_retrieve(label, property);
      if (idx) _index_InsertNode(idx, id, &current->properties[j].value);
    }
  }
}


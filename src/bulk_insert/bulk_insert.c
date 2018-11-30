/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "bulk_insert.h"
#include "../stores/store.h"
#include <errno.h>
#include <assert.h>
#include <libgen.h>

typedef enum {
  BI_NULL,
  BI_BOOL,
  BI_NUMERIC,
  BI_STRING,
  BI_INT // Count/length
} TYPE;

// For debug printing
char *types[] =  {"BI_NULL", "BOOL", "NUMERIC", "STRING"};

int _BulkInsert_ProcessNodeFile(RedisModuleCtx *ctx, GraphContext *gc, char *filename) {
  FILE *label_file = fopen(filename, "rb");
  // Handle failure to open provided filename
  if (label_file == NULL) {
    // TODO tmp
    perror("Error while parsing label: ");
    return BULK_FAIL;
  }

  // TODO don't use constants
  unsigned char data[10240];
  size_t num_read = fread(data, 1, 10240, label_file);
  if (ferror(label_file)) {

    char *error;
    asprintf(&error, "Failure reading header data from file '%s'\n", filename);
    RedisModule_ReplyWithError(ctx, error);
    free(error);
  }

  int data_idx = 0;
  int len;

  // Read the file header
  int prop_count;
  // TODO don't use constants
  char *prop_keys[1024];
  // First 4 bytes are label length
  len = *(int*)data;
  data_idx += sizeof(int);

  // Next sequence is label string
  char label[len + 1];
  memcpy(label, data + data_idx, len);
  label[len] = 0; // add null terminator
  data_idx += len;
  LabelStore *store = GraphContext_AddLabel(gc, label);

  // Next 4 bytes are property count
  prop_count = *(int*)&data[data_idx];
  data_idx += sizeof(int);
  // The rest of the line is [int len, char *prop_key] * prop_count
  for (int j = 0; j < prop_count; j ++) {
    len = *(int*)&data[data_idx];
    data_idx += sizeof(int);

    prop_keys[j] = malloc((len + 1) * sizeof(char));
    memcpy(prop_keys[j], data + data_idx, len);
    prop_keys[j][len] = 0; // add null terminator
    data_idx += len;
    // printf("prop key %d: %s\n", j, prop_keys[j]);
  }

  // Add properties to schemas
  LabelStore_UpdateSchema(GraphContext_AllStore(gc, STORE_NODE), prop_count, prop_keys);
  LabelStore_UpdateSchema(store, prop_count, prop_keys);

  // Entity handling
  // Reusable buffer for storing properties of each node
  SIValue values[prop_count];

  int prop_num = 0;
  while (data_idx < num_read) {
    // TODO get more data if necessary
    Node n;
    Graph_CreateNode(gc->g, store->id, &n);
    for (int i = 0; i < prop_count; i ++) {
      TYPE t = data[data_idx++];
      // printf("%s: ", types[t]);
      if (t == BI_NULL) {
        values[i] = SI_NullVal(); // TODO ensure this property doesn't get added
      } else if (t == BI_BOOL) {
        bool b = data[data_idx++];
        values[i] = SI_BoolVal(b);
        // printf("%s\n", v.boolval ? "true" : "false");
      } else if (t == BI_NUMERIC) {
        double d = *(double*)&data[data_idx];
        data_idx += sizeof(double);
        values[i] = SI_DoubleVal(d);
        // printf("%f\n", v.doubleval);
      } else if (t == BI_STRING) {
        // Read string length
        len = *(int*)&data[data_idx];
        data_idx += sizeof(int);
        char *s = malloc((len + 1) * sizeof(char));
        memcpy(s, data + data_idx, len);
        s[len] = 0; // add null terminator
        data_idx += len;
        // printf("%s\n", v.stringval);
        values[i] = SI_TransferStringVal(s);
      } else {
        assert(0);
      }
    }
    GraphEntity_Add_Properties((GraphEntity*)&n, prop_count, prop_keys, values);
    // printf("%s\n", data);
  }

  fclose(label_file);

  return BULK_OK;
}

int _BulkInsert_ProcessRelationFile(RedisModuleCtx *ctx, GraphContext *gc, char *filename) {
  FILE *reltype_file = fopen(filename, "rb");
  // Handle failure to open provided filename
  if (reltype_file == NULL) {
    // TODO tmp
    perror("Error while parsing relation: ");
    return BULK_FAIL;
  }

  // TODO don't use constants
  unsigned char data[10240];
  size_t num_read = fread(data, 1, 10240, reltype_file);
  if (ferror(reltype_file)) {
    char *error;
    asprintf(&error, "Failure reading header data from file '%s'\n", filename);
    RedisModule_ReplyWithError(ctx, error);
    free(error);
  }

  int data_idx = 0;
  int len;

  // Read the file header
  int prop_count;
  // TODO don't use constants
  char *prop_keys[1024];
  // First 4 bytes are relation type length
  len = *(int*)data;
  data_idx += sizeof(int);

  // Next sequence is reltype string
  char reltype[len + 1];
  memcpy(reltype, data + data_idx, len);
  reltype[len] = 0; // add null terminator
  data_idx += len;
  LabelStore *store = GraphContext_AddRelationType(gc, reltype);

  // Next 4 bytes are property count
  prop_count = *(int*)&data[data_idx];
  data_idx += sizeof(int);
  // The rest of the line is [int len, char *prop_key] * prop_count
  // This loop won't execute if relations do not have properties.
  for (int j = 0; j < prop_count; j ++) {
    len = *(int*)&data[data_idx];
    data_idx += sizeof(int);

    prop_keys[j] = malloc((len + 1) * sizeof(char));
    memcpy(prop_keys[j], data + data_idx, len);
    prop_keys[j][len] = 0; // add null terminator
    data_idx += len;
    // printf("prop key %d: %s\n", j, prop_keys[j]);
  }

  // Add properties to schemas
  LabelStore_UpdateSchema(GraphContext_AllStore(gc, STORE_EDGE), prop_count, prop_keys);
  LabelStore_UpdateSchema(store, prop_count, prop_keys);

  // Entity handling
  // Reusable buffer for storing properties of each relation 
  SIValue values[prop_count];

  int prop_num = 0;
  NodeID src;
  NodeID dest;

  while (data_idx < num_read) {
    // TODO get more data if necessary
    Edge e;
    // Graph_CreateEdge(gc->g, &e);

    // Next 8 bytes are source ID
    src = *(NodeID*)&data[data_idx];
    data_idx += sizeof(NodeID);
    // Next 8 bytes are destination ID
    dest = *(NodeID*)&data[data_idx];
    data_idx += sizeof(NodeID);

    for (int i = 0; i < prop_count; i ++) {
      TYPE t = data[data_idx++];
      // printf("%s: ", types[t]);
      if (t == BI_NULL) {
        values[i] = SI_NullVal(); // TODO ensure this property doesn't get added
      } else if (t == BI_BOOL) {
        bool b = data[data_idx++];
        values[i] = SI_BoolVal(b);
        // printf("%s\n", v.boolval ? "true" : "false");
      } else if (t == BI_NUMERIC) {
        double d = *(double*)&data[data_idx];
        data_idx += sizeof(double);
        values[i] = SI_DoubleVal(d);
        // printf("%f\n", v.doubleval);
      } else if (t == BI_STRING) {
        // Read string length
        len = *(int*)&data[data_idx];
        data_idx += sizeof(int);
        char *s = malloc((len + 1) * sizeof(char));
        memcpy(s, data + data_idx, len);
        s[len] = 0; // add null terminator
        data_idx += len;
        // printf("%s\n", v.stringval);
        values[i] = SI_TransferStringVal(s);
      } else {
        assert(0);
      }
    }
    Graph_ConnectNodes(gc->g, src, dest, store->id, &e);
    GraphEntity_Add_Properties((GraphEntity*)&e, prop_count, prop_keys, values);
    // printf("%s\n", data);
  }

  fclose(reltype_file);

  return BULK_OK;
}

int _BulkInsert_InsertNodes(RedisModuleCtx *ctx, GraphContext *gc,
                             RedisModuleString ***argv, int *argc) {
    int rc;
    while (argv && *argc) {
      char *filename = strdup(RedisModule_StringPtrLen(**argv, NULL));
      // TODO should free argument
      // Done processing labels
      if (!strcmp(filename, "RELATIONS")) {
        return BULK_OK;
      }
      *argv += 1;
      *argc -= 1;
      rc = _BulkInsert_ProcessNodeFile(ctx, gc, filename);
      assert (rc == BULK_OK);
    }
    return BULK_OK;
}

int _BulkInsert_Insert_Edges(RedisModuleCtx *ctx, GraphContext *gc,
                             RedisModuleString ***argv, int *argc) {
    int rc;
    while (argv && *argc) {
      char *filename = strdup(RedisModule_StringPtrLen(**argv, NULL));
      // TODO should free argument
      *argv += 1;
      *argc -= 1;
      rc = _BulkInsert_ProcessRelationFile(ctx, gc, filename);
      assert (rc == BULK_OK);
    }
    return BULK_OK;
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, RedisModuleString **argv, int argc) {

    if(argc < 1) {
        RedisModule_ReplyWithError(ctx, "Bulk insert format error, failed to parse bulk insert sections.");
        return BULK_FAIL;
    }

    bool section_found = false;
    const char *section = RedisModule_StringPtrLen(*argv++, NULL);
    argc -= 1;

    int rc;
    //TODO: Keep track and validate argc, make sure we don't overflow.
    if(strcmp(section, "NODES") == 0) {
        section_found = true;
        rc = _BulkInsert_InsertNodes(ctx, gc, &argv, &argc);
        if (rc != BULK_OK) {
            return BULK_FAIL;
        } else if (argc == 0) {
            return BULK_OK;
        }
        section = RedisModule_StringPtrLen(*argv++, NULL);
        argc -= 1;
    }

    if(strcmp(section, "RELATIONS") == 0) {
        section_found = true;
        rc = _BulkInsert_Insert_Edges(ctx, gc, &argv, &argc);
        if (rc != BULK_OK) {
            return BULK_FAIL;
        } else if (argc == 0) {
            return BULK_OK;
        }
        section = RedisModule_StringPtrLen(*argv++, NULL);
        argc -= 1;
        assert (argc == 0);
    }

    if (!section_found) {
        char *error;
        asprintf(&error, "Unexpected token %s, expected NODES or RELATIONS.", section);
        RedisModule_ReplyWithError(ctx, error);
        free(error);
        return BULK_FAIL;
    }

    if(argc > 0) {
        RedisModule_ReplyWithError(ctx, "Bulk insert format error, extra arguments.");
        return BULK_FAIL;
    }

    return BULK_OK;
}


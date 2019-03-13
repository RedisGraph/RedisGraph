/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "serialize_schema.h"
#include "../../util/arr.h"

/* Deserialize schema */
Schema* RdbLoadUnifiedSchema(RedisModuleIO *rdb, TrieMap *attributes, char **string_mapping) {
  /* Format:
   * id
   * name
   * #attributes
   * attributes
   */

  int id = RedisModule_LoadUnsigned(rdb);
  char *name = RedisModule_LoadStringBuffer(rdb, NULL);
  Schema *s = Schema_New(name, id);

  size_t len = 0;
  uint64_t attrCount = RedisModule_LoadUnsigned(rdb);


  for(uint i = 0; i < attrCount; i++) {
    // Load attribute string from RDB file.
    char *attr = RedisModule_LoadStringBuffer(rdb, &len);

    // Add the string directly to the schema triemap using the RDB-given length.
    Attribute_ID *pAttribute_id = NULL;
    pAttribute_id = malloc(sizeof(Attribute_ID));
    *pAttribute_id = i;

    TrieMap_Add(attributes, attr, len, pAttribute_id, TrieMap_DONT_CARE_REPLACE);
    string_mapping[i] = attr; // TODO strings need to be freed elsewhere 
  }

  return s;
}

Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
  /* Format:
   * id
   * name
   * #attributes
   * attributes
   */

  int id = RedisModule_LoadUnsigned(rdb);
  char *name = RedisModule_LoadStringBuffer(rdb, NULL);
  Schema *s = Schema_New(name, id);

  size_t len = 0;
  uint64_t attrCount = RedisModule_LoadUnsigned(rdb);

  char attribute[1024];

  // Only doing this for backwards compatibility; we're not keeping these strings
  for(uint i = 0; i < attrCount; i++) {
    // Load attribute string from RDB file.
    char *attr = RedisModule_LoadStringBuffer(rdb, &len);
    // Immediately free the string, as the schema does not reference it.
    RedisModule_Free(attr);
  }

  return s;
}

void RdbSaveSchema(RedisModuleIO *rdb, void *value) {
  /* Format:
   * id
   * name
   * #attributes
   * attributes
   */

  Schema *s = value;

  RedisModule_SaveUnsigned(rdb, s->id);
  RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);
  // Don't save any strings
  RedisModule_SaveUnsigned(rdb, 0);
}

/* Serialize schema */
void RdbSaveUnifiedSchema(RedisModuleIO *rdb, void *value, const char **string_mapping) {
  /* Format:
   * id
   * name
   * #attributes
   * attributes
   */

  Schema *s = value;

  RedisModule_SaveUnsigned(rdb, s->id);
  RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);
  uint len = array_len(string_mapping);
  RedisModule_SaveUnsigned(rdb, len);
  // TODO edges overwrite nodes, etc. Might need to be breaking?
  for (uint i = 0; i < len; i ++) {
    RedisModule_SaveStringBuffer(rdb, string_mapping[i], len);
  }
}

/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "serialize_schema.h"

/* Deserialize schema */
Schema* RdbLoadUnifiedSchema(RedisModuleIO *rdb) {
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

  for(int i = 0; i < attrCount; i++) {
    // Load attribute string from RDB file.
    char *attr = RedisModule_LoadStringBuffer(rdb, &len);

    // Add the string directly to the schema triemap using the RDB-given length.
    Attribute_ID *pAttribute_id = NULL;
    pAttribute_id = malloc(sizeof(Attribute_ID));
    *pAttribute_id = i;

    TrieMap_Add(s->attributes, attr, len, pAttribute_id, TrieMap_DONT_CARE_REPLACE);

    // Immediately free the string, as the schema does not reference it.
    RedisModule_Free(attr);
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

  for(int i = 0; i < attrCount; i++) {
    // Load attribute string from RDB file.
    char *attr = RedisModule_LoadStringBuffer(rdb, &len);
    
    // TODO: make sure we do not overflow.
    memcpy(attribute, attr, len);
    attribute[len] = 0;

    // Attribute ID will be retrieved from unified schema.
    Schema_AddAttribute(s, type, attribute);

    // Immediately free the string, as the schema does not reference it.
    RedisModule_Free(attr);
  }

  return s;
}

/* Serialize schema */
void RdbSaveSchema(RedisModuleIO *rdb, void *value) {
  /* Format:
   * id
   * name
   * #attributes
   * attributes
   */

  Schema *s = value;

  unsigned short schema_attr_count = Schema_AttributeCount(s);
  RedisModule_SaveUnsigned(rdb, s->id);
  RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);
  RedisModule_SaveUnsigned(rdb, schema_attr_count);

  if(schema_attr_count) {
    char *ptr;
    tm_len_t len;
    void *v;
    TrieMapIterator *it = TrieMap_Iterate(s->attributes, "", 0);
    while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
      RedisModule_SaveStringBuffer(rdb, ptr, len);
    }
    TrieMapIterator_Free(it);
  }
}

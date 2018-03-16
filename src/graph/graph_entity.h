#ifndef GRAPH_ENTITY_H_
#define GRAPH_ENTITY_H_

#include "../value.h"
#define INVALID_ENTITY_ID -1l


SIValue *PROPERTY_NOTFOUND;

typedef struct {
    char *name;
    SIValue value;
} EntityProperty;

typedef struct {
    long int id;                    /* unique id (might be empty) */
    int prop_count;
    EntityProperty *properties;
} GraphEntity;

/* Adds a properties to entity
 * prop_count - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void GraphEntity_Add_Properties(GraphEntity *e, int prop_count, char **keys, SIValue *values);

/* Retrieves entity's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue* GraphEntity_Get_Property(const GraphEntity *e, const char* key);

/* Release all memory allocated by entity */
void FreeGraphEntity(GraphEntity *e);

#endif

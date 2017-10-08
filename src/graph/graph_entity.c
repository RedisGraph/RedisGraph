#include "graph.h"
#include "graph_entity.h"

SIValue *PROPERTY_NOTFOUND = NULL;

/* Expecting e to be either *Node or *Edge */
void GraphEntity_Add_Properties(GraphEntity *e, int prop_count, char **keys, SIValue *values) {
	if(e->properties == NULL) {
		e->properties = malloc(sizeof(EntityProperty) * prop_count);
	} else {
		e->properties = realloc(e->properties, sizeof(EntityProperty) * (e->prop_count + prop_count));
	}

	for(int i = 0; i < prop_count; i++) {
		e->properties[e->prop_count + i].name = keys[i];
		e->properties[e->prop_count + i].value = values[i];
	}

	e->prop_count += prop_count;
}

SIValue* GraphEntity_Get_Property(const GraphEntity *e, const char* key) {
	for(int i = 0; i < e->prop_count; i++) {
		if(strcmp(key, e->properties[i].name) == 0) {
			return &e->properties[i].value;
		}
	}
	return PROPERTY_NOTFOUND;
}

void FreeGraphEntity(GraphEntity *e) {
	if(e->properties == NULL) {
		for(int i = 0; i < e->prop_count; i++) {
			free(e->properties[i].name);
		}		
		free(e->properties);
	}
}
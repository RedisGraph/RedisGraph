#include "constraint.h"
#include "../util/arr.h"
#include "../graph/entities/attribute_set.h"
#include "../index/index.h"
#include "redisearch_api.h"
#include "../index/index.h"
#include "value.h"


Constraint Constraint_new(ConstAttrData *attrData, uint id_count, const char *label, int label_id, 
GraphEntityType type) {    
    Constraint c = rm_malloc(sizeof(_Constraint));
    c->attributes = array_new(ConstAttrData, id_count);

    memcpy(c->attributes, attrData, sizeof(ConstAttrData) * id_count);
    for(int i = 0; i < id_count; i++) {
        c->attributes[i].attribute_name = rm_strdup(attrData[i].attribute_name);
    }
    c->label = rm_strdup(label);
    c->label_id = label_id;
    c->entity_type = type;
    return c;
}

Constraint Constraint_free() {
    for(int i = 0; i < array_len(c->attributes); i++) {
        rm_free(c->attributes[i].attribute_name);
    }
    array_free(c->attributes);
    rm_free(c->label);
    rm_free(c);
}

// check if entity constains all attributes of the constraint
static bool _Should_constraint_enforce_entity(Constraint c, const AttributeSet attributes) {
    size_t len = array_len(c->ids);
    for(size_t i = 0; i < len; ++i) {
        if(AttributeSet_Get(attributes, c->ids[i]) == ATTRIBUTE_NOTFOUND) {
            return false;
        }
    }

    return true;
}

bool Constraint_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx) {
    if(!_Should_constraint_enforce_entity(c, attributes)) {
        return true;
    }

    SIValue *v;
    SIType t;
    RSQNode  *node =  NULL;
    // convert constraint into a RediSearch query
    RSQNode *rs_query_node = RediSearch_CreateIntersectNode(idx, false);
    ASSERT(rs_query_node != NULL);

    for(uint i = 0; i < array_len(c->ids); i++) {
        char *field = c->attributes[i].attribute_name;
        v = AttributeSet_Get(attributes, c->attributes[i].id);
        ASSERT(v != ATTRIBUTE_NOTFOUND); // We already ensured it using _Should_constraint_enforce_entity
        t = SI_TYPE(*v);

	    if(!(t & SI_INDEXABLE)) {
	    	// none indexable type, consult with the none indexed field
	    	node = RediSearch_CreateTagNode(idx, INDEX_FIELD_NONE_INDEXED);
	    	RSQNode *child = RediSearch_CreateTokenNode(idx,
	    			INDEX_FIELD_NONE_INDEXED, field);

	    	RediSearch_QueryNodeAddChild(node, child);
	    } else if(t == T_STRING) {
            node = RediSearch_CreateTagNode(idx, field);
            RSQNode *child = RediSearch_CreateTokenNode(idx, field, v.stringval);
	    	RediSearch_QueryNodeAddChild(node, child);
        } else {
            ASSERT(t & SI_NUMERIC || t == T_BOOL);
		    double d = SI_GET_NUMERIC(v);
            node = RediSearch_CreateNumericNode(idx, c->attribute_name, d, d, true, true);
        }
        ASSERT(node != NULL);
        RediSearch_IntersectNodeAddChild(rs_query_node, node);
    }

    RSResultsIterator *iter = RediSearch_GetResultsIterator(rs_query_node, idx);
    void* ptr = RediSearch_ResultsIteratorNext(iter, idx, NULL);
    ASSERT(ptr != NULL);
    ptr = RediSearch_ResultsIteratorNext(iter, idx, NULL);
    if(ptr != NULL) {
        // We have the same property value twice.
        return false;
    }

    RediSearch_ResultsIteratorFree(iter);
    return true;
}

bool Constraints_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx) {
    for(int i = 0; i < array_len(c); i++) {
        if(!Constraint_enforce_entity(c[i], attributes, idx)) {
            return false;
        }
    }

    return true;
}

void Free_Constraint_Remove_Its_Index(Constraint c, const GraphContext *gc) {
	// constraint was not satisfied, free it and remove it's index

	SchemaType schema_type = (c->entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
	QueryCtx_LockForCommit();
	Schema *s = GraphContext_GetSchema(gc, c->label, schema_type);
	ASSERT(s);

	// remove all index fields
	for(int i = 0; i < array_len(c->attribute_name); i++) {
		int res = GraphContext_DeleteIndex(gc, schema_type, c->label, c->attribute_name,
			IDX_EXACT_MATCH, true);
		ASSERT(res != INDEX_FAIL); // index should exist

		if(res == INDEX_OK) {
			Index idx = Schema_GetIndex(s, c->ids[i], IDX_EXACT_MATCH);
			ASSERT(*idx != NULL);

			if(Index_FieldsCount(idx) > 0) {
				Indexer_PopulateIndexOrConstraint(gc, idx, NULL);
			} else {
				Indexer_DropIndex(idx);
			}
		}
	}
	constraint_free(c);
	QueryCtx_UnlockCommit(NULL);
}

bool Has_Constraint_On_Attribute(const Constraint constraints, Attribute_ID attr_id) {
    for(int i = 0; i < array_len(constraints); i++) {
        _Constraint c = constraints[i];
        for(int j = 0; j < array_len(c->attributes); j++) {
            if(c->attributes[j].id == attr_id) {
                return true;
            }
        }
    }

    return false;
}

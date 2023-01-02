#include "constraint.h"
#include "../util/arr.h"
#include "../graph/entities/attribute_set.h"
#include "../index/index.h"
#include "redisearch_api.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "value.h"
#include "../index/indexer.h"
#include "../query_ctx.h"
#include <stdatomic.h>


const ConstAttrData *Constraint_GetAttributes
(
	const Constraint c
) {
	ASSERT(c != NULL);

	return (const ConstAttrData *)c->attributes;
}

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
    c->status = CT_PENDING;
    c->pending_changes = ATOMIC_VAR_INIT(0);
    return c;
}

void Constraint_Activate(Constraint c) {
    c->status = CT_ACTIVE;
}

// returns number of pending changes
int Constraint_PendingChanges
(
	const Constraint c  // constraint to inquery
) {
	ASSERT(c != NULL);
    ASSERT(c->pending_changes <= 2); // one can't create or drop the same constraint twice

	return c->pending_changes;
}

// increment number of pending changes
void Constraint_IncPendingChanges
(
	Constraint c
) {
	ASSERT(c != NULL);
    ASSERT(c->pending_changes <= 2); // one can't create or drop the same constraint twice

	c->pending_changes++;
}

// decrement number of pending changes
void Constraint_DecPendingChanges
(
	Constraint c
) {
	ASSERT(c != NULL);
    ASSERT(c->pending_changes > 0);
    ASSERT(c->pending_changes <= 2); // one can't create or drop the same constraint twice

	c->pending_changes--;
}

void Constraint_free(Constraint c) {
    for(int i = 0; i < array_len(c->attributes); i++) {
        rm_free(c->attributes[i].attribute_name);
    }
    array_free(c->attributes);
    rm_free(c->label);
    rm_free(c);
}

// check if entity constains all attributes of the constraint
static bool _Should_constraint_enforce_entity(Constraint c, const AttributeSet attributes) {
    size_t len = array_len(c->attributes);
    for(size_t i = 0; i < len; ++i) {
        if(AttributeSet_Get(attributes, c->attributes[i].id) == ATTRIBUTE_NOTFOUND) {
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

    for(uint i = 0; i < array_len(c->attributes); i++) {
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
            RSQNode *child = RediSearch_CreateTokenNode(idx, field, v->stringval);
	    	RediSearch_QueryNodeAddChild(node, child);
        } else {
            ASSERT(t & SI_NUMERIC || t == T_BOOL);
		    double d = SI_GET_NUMERIC((*v));
            node = RediSearch_CreateNumericNode(idx, field, d, d, true, true);
        }
        ASSERT(node != NULL);
        RediSearch_QueryNodeAddChild(rs_query_node, node);
    }

    RSResultsIterator *iter = RediSearch_GetResultsIterator(rs_query_node, idx);
    const void* ptr = RediSearch_ResultsIteratorNext(iter, idx, NULL);
    ASSERT(ptr != NULL);
    ptr = RediSearch_ResultsIteratorNext(iter, idx, NULL);
    if(ptr != NULL) {
        // We have the same property value twice.
        return false;
    }

    RediSearch_ResultsIteratorFree(iter);
    return true;
}

bool Constraints_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx, int *ind) {
    for(int i = 0; i < array_len(c); i++) {
        if(c[i].status != CT_FAILED && !Constraint_enforce_entity(&c[i], attributes, idx)) {
            if(ind) *ind = i;
            return false;
        }
    }

    return true;
}

// Executed under write lock
void Constraint_Drop_Index(Constraint c, struct GraphContext *gc, bool should_drop_constraint) {
	// constraint was not satisfied, free it and remove it's index

	SchemaType schema_type = (c->entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
	Schema *s = GraphContext_GetSchema((GraphContext*)gc, c->label, schema_type);
	ASSERT(s);

	// remove all index fields
	for(int i = 0; i < array_len(c->attributes); i++) {
		int res = GraphContext_DeleteIndex((GraphContext*)gc, schema_type, c->label, c->attributes[i].attribute_name,
			IDX_EXACT_MATCH, true);
		ASSERT(res != INDEX_FAIL); // index should exist

		if(res == INDEX_OK) {
			Index idx = Schema_GetIndex(s, &(c->attributes[i].id), IDX_EXACT_MATCH);
			ASSERT(idx != NULL);

			if(Index_FieldsCount(idx) > 0) {
				Indexer_PopulateIndexOrConstraint((GraphContext*)gc, idx, NULL);
			} else {
                if(should_drop_constraint) {
                    Constraint_IncPendingChanges(c);
                }
				Indexer_DropIndexOrConstraint(idx, should_drop_constraint ? c : NULL);
			}
		}
	}
}

bool Has_Constraint_On_Attribute(const Constraint constraints, Attribute_ID attr_id) {
    for(int i = 0; i < array_len(constraints); i++) {
        _Constraint c = constraints[i];
        for(int j = 0; j < array_len(c.attributes); j++) {
            if(c.attributes[j].id == attr_id) {
                return true;
            }
        }
    }

    return false;
}

typedef enum {
	CT_CREATE,
	CT_DELETE
} ConstraintOp;

static int Constraint_Parse(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleString **graph_name, GraphEntityType *type, 
                            const char **label, RedisModuleString ***props, long long *prop_count, ConstraintOp *op, ConstraintType *ct) {
	// get graph name
	argv += 1; // skip "GRAPH.CONSTRAINT"

    const char *token = RedisModule_StringPtrLen(*argv++, NULL);
    if(strcasecmp(token, "CREATE") == 0) {
        *op = CT_CREATE;
    } else if(strcasecmp(token, "DELETE") == 0) {
        *op = CT_DELETE;
    } else {
        RedisModule_ReplyWithError(ctx, "invalid constraint operation");
        return REDISMODULE_ERR;
    }

	*graph_name = *argv++;
    token = RedisModule_StringPtrLen(*argv++, NULL);
    if(strcasecmp(token, "UNIQUE") == 0) {
        *ct = CT_UNIQUE;
    } else {
        RedisModule_ReplyWithError(ctx, "invalid constraint type");
        return REDISMODULE_ERR; //currently only unique constraint is supported
    }

    token = RedisModule_StringPtrLen(*argv++, NULL);
    if(strcasecmp(token, "LABEL") == 0) {
        *type = GETYPE_NODE;
    } else if(strcasecmp(token, "RELTYPE") == 0) {
        *type = GETYPE_EDGE;
    } else {
        RedisModule_ReplyWithError(ctx, "invalid constraint entity type");
        return REDISMODULE_ERR;
    }

    *label = RedisModule_StringPtrLen(*argv++, NULL);
    token = RedisModule_StringPtrLen(*argv++, NULL);
    if(strcasecmp(token, "PROPERTIES") == 0) {
        if (RedisModule_StringToLongLong(*argv++, prop_count) != REDISMODULE_OK || *prop_count < 1) {
            RedisModule_ReplyWithError(ctx, "invalid property count");
            return REDISMODULE_ERR;
        }
    } else {
        RedisModule_ReplyWithError(ctx, "7th arg isn't PROPERTIES");
        return REDISMODULE_ERR;
    }

    if(argc - 8 != *prop_count) {
        RedisModule_ReplyWithError(ctx, "Number of properties doesn't match property count");
        return REDISMODULE_ERR;
    }

    *props = argv;

    return REDISMODULE_OK;
}

int Graph_Constraint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc < 7) return RedisModule_WrongArity(ctx);

	GraphContext *gc = NULL;
    RedisModuleString *rs_graph_name;
    GraphEntityType entity_type;
    const char *label;
    RedisModuleString **props;
    long long prop_count;
    ConstraintOp op;
    ConstraintType ct;
    int rv = REDISMODULE_OK;

    if(Constraint_Parse(ctx, argv, argc, &rs_graph_name, &entity_type, &label, &props, &prop_count, &op, &ct) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    const char *props_cstr[prop_count];
    for(int i = 0; i < prop_count; i++) {
        props_cstr[i] = RedisModule_StringPtrLen(props[i], NULL);
    }

	gc = GraphContext_Retrieve(ctx, rs_graph_name, false, false);
    if(!gc) {
        RedisModule_ReplyWithError(ctx, "Invalid graph name passed as argument");
        return REDISMODULE_ERR;
    }

    SchemaType schema_type = (entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;

    // lock
	QueryCtx_LockForCommit();
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
    if(op == CT_DELETE && !s) {
        RedisModule_ReplyWithError(ctx, "Schema not found");
        rv =  REDISMODULE_ERR;
        goto _out;
    }

	Index idx = NULL;
	Constraint c = NULL;

    if(op == CT_CREATE) {

        c = GraphContext_AddUniqueConstraint(gc, schema_type, label, props_cstr, prop_count);

        if(!c) { // constraint already exists
            RedisModule_ReplyWithError(ctx, "Constraint already exists");
            rv = REDISMODULE_ERR;
            goto _out;
        }

        // add fields to index
        GraphContext_AddExactMatchIndex(&idx, gc, schema_type, label, props_cstr, prop_count);

        Indexer_PopulateIndexOrConstraint(gc, idx, c);

    } else { // CT_DELETE
        ConstAttrData fields[prop_count];

        for(int i = 0; i < prop_count; i++) {
            fields[i].attribute_name = (char *)props_cstr[i];
        }

        Constraint c = Schema_GetConstraint(s, fields, prop_count);
        if(!c) {
            RedisModule_ReplyWithError(ctx, "Constraint not found");
            rv = REDISMODULE_ERR;
            goto _out;
        }

        Schema_RemoveConstraint(s, c);
        Constraint_Drop_Index(c, (struct GraphContext *)gc, true);
    }

_out:
    // unlock
    QueryCtx_UnlockCommit();
    // decrease graph reference count
	GraphContext_DecreaseRefCount(gc);
    return rv;    
}

#include "ast_shared.h"
#include "../RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../arithmetic/arithmetic_expression_construct.h"

AST_Operator AST_ConvertOperatorNode(const cypher_operator_t *op) {
	// TODO ordered by precedence, which I don't know if we're managing properly right now
	if(op == CYPHER_OP_OR) {
		return OP_OR;
	} else if(op == CYPHER_OP_XOR) {
		return OP_XOR;
	} else if(op == CYPHER_OP_AND) {
		return OP_AND;
	} else if(op == CYPHER_OP_NOT) {
		// Unary, maybe doesn't belong here
		return OP_NOT;
	} else if(op == CYPHER_OP_EQUAL) {
		return OP_EQUAL;
	} else if(op == CYPHER_OP_NEQUAL) {
		return OP_NEQUAL;
	} else if(op == CYPHER_OP_LT) {
		return OP_LT;
	} else if(op == CYPHER_OP_GT) {
		return OP_GT;
	} else if(op == CYPHER_OP_LTE) {
		return OP_LE;
	} else if(op == CYPHER_OP_GTE) {
		return OP_GE;
	} else if(op == CYPHER_OP_PLUS) {
		return OP_PLUS;
	} else if(op == CYPHER_OP_MINUS) {
		return OP_MINUS;
	} else if(op == CYPHER_OP_MULT) {
		return OP_MULT;
	} else if(op == CYPHER_OP_DIV) {
		return OP_DIV;
	} else if(op == CYPHER_OP_MOD) {
		return OP_MOD;
	} else if(op == CYPHER_OP_POW) {
		return OP_POW;
	} else if(op == CYPHER_OP_CONTAINS) {
		return OP_CONTAINS;
	} else if(op == CYPHER_OP_STARTS_WITH) {
		return OP_STARTSWITH;
	} else if(op == CYPHER_OP_ENDS_WITH) {
		return OP_ENDSWITH;
	} else if(op == CYPHER_OP_IN) {
		return OP_IN;
	} else if(op == CYPHER_OP_IS_NULL) {
		return OP_IS_NULL;
	} else if(op == CYPHER_OP_IS_NOT_NULL) {
		return OP_IS_NOT_NULL;
	}

	return -1;
}

PropertyMap *PropertyMap_New(GraphContext *gc, const cypher_astnode_t *props) {
	if(props == NULL) return NULL;
	ASSERT(cypher_astnode_type(props) == CYPHER_AST_MAP); // TODO add parameter support

	uint prop_count = cypher_ast_map_nentries(props);

	PropertyMap *map = rm_malloc(sizeof(PropertyMap));
	map->keys = array_new(Attribute_ID, prop_count);
	map->values = array_new(AR_ExpNode *, prop_count);

	for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
		uint insert_idx                   = prop_idx;
		const cypher_astnode_t *ast_key   = cypher_ast_map_get_key(props, prop_idx);
		const char *attribute             = cypher_ast_prop_name_get_value(ast_key);
		const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
		AR_ExpNode *value                 = AR_EXP_FromASTNode(ast_value);

		// Convert the string key to an Attribute ID.
		Attribute_ID id = GraphContext_FindOrAddAttribute(gc, attribute);
		// search for duplicate attributes
		uint count = array_len(map->keys);
		for (uint i = 0; i < count; i++) {
			if(map->keys[i] == id) {
				insert_idx = i;
				break;
			}
		}

		if(insert_idx == prop_idx) {
			array_append(map->keys, id);			
			array_append(map->values, value);
		} else {
			AR_EXP_Free(map->values[insert_idx]);
			map->values[insert_idx] = value;
		}
	}

	return map;
}

static PropertyMap *_PropertyMap_Clone(PropertyMap *map) {
	PropertyMap *clone = rm_malloc(sizeof(PropertyMap));
	array_clone(clone->keys, map->keys);
	array_clone_with_cb(clone->values, map->values, AR_EXP_Clone);

	return clone;
}

void PropertyMap_Free(PropertyMap *map) {
	if(map == NULL) return;

	array_free(map->keys);
	array_free_cb(map->values, AR_EXP_Free);
	rm_free(map);
}

NodeCreateCtx NodeCreateCtx_Clone(NodeCreateCtx ctx) {
	NodeCreateCtx clone = ctx;
	array_clone(clone.labels, ctx.labels);
	array_clone(clone.labelsId, ctx.labelsId);
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}

void NodeCreateCtx_Free(NodeCreateCtx ctx) {
	array_free(ctx.labels);
	array_free(ctx.labelsId);
	PropertyMap_Free(ctx.properties);
}

EdgeCreateCtx EdgeCreateCtx_Clone(EdgeCreateCtx ctx) {
	EdgeCreateCtx clone = ctx;
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}

EntityUpdateEvalCtx *UpdateCtx_New(UPDATE_MODE mode, uint prop_count, const char *alias) {
	EntityUpdateEvalCtx *ctx = rm_malloc(sizeof(EntityUpdateEvalCtx));
	ctx->mode = mode;
	ctx->alias = alias;
	ctx->record_idx = INVALID_INDEX;
	ctx->labels = array_new(const char *, prop_count);
	ctx->properties = array_new(PropertySetCtx, prop_count);

	return ctx;
}

EntityUpdateEvalCtx *UpdateCtx_Clone(const EntityUpdateEvalCtx *orig) {
	EntityUpdateEvalCtx *clone = rm_malloc(sizeof(EntityUpdateEvalCtx));
	clone->mode = orig->mode;
	clone->alias = orig->alias;
	clone->record_idx = orig->record_idx;
	uint count = array_len(orig->properties);
	array_clone(clone->labels, orig->labels);
	clone->properties = array_new(PropertySetCtx, count);
	for(uint i = 0; i < count; i ++) {
		PropertySetCtx update = {
			.id = orig->properties[i].id,
			.exp = AR_EXP_Clone(orig->properties[i].exp),
		};
		array_append(clone->properties, update);
	}
	return clone;
}

void UpdateCtx_SetMode(EntityUpdateEvalCtx *ctx, UPDATE_MODE mode) {
	ASSERT(ctx != NULL);
	ctx->mode = mode;
}

void UpdateCtx_Clear(EntityUpdateEvalCtx *ctx) {
	uint count = array_len(ctx->properties);
	for(uint i = 0; i < count; i ++) AR_EXP_Free(ctx->properties[i].exp);
	array_clear(ctx->properties);
}

void UpdateCtx_Free(EntityUpdateEvalCtx *ctx) {
	uint count = array_len(ctx->properties);
	for(uint i = 0; i < count; i ++) AR_EXP_Free(ctx->properties[i].exp);
	array_free(ctx->properties);
	array_free(ctx->labels);

	rm_free(ctx);
}


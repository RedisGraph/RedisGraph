/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast_shared.h"
#include "../RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"
#include "../arithmetic/arithmetic_expression_construct.h"

AST_Operator AST_ConvertOperatorNode
(
	const cypher_operator_t *op
) {
	// TODO: ordered by precedence
	// which I don't know if we're managing properly right now
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

PropertyMap *PropertyMap_New
(
	GraphContext *gc,
	const cypher_astnode_t *props
) {
	if(props == NULL) return NULL;
	ASSERT(cypher_astnode_type(props) == CYPHER_AST_MAP); // TODO add parameter support

	uint prop_count = cypher_ast_map_nentries(props);

	PropertyMap *map = rm_malloc(sizeof(PropertyMap));
	map->keys = array_new(const char *, prop_count);
	map->values = array_new(AR_ExpNode *, prop_count);

	for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
		uint insert_idx                   = prop_idx;
		const cypher_astnode_t *ast_key   = cypher_ast_map_get_key(props, prop_idx);
		const char *attribute             = cypher_ast_prop_name_get_value(ast_key);
		const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
		AR_ExpNode *value                 = AR_EXP_FromASTNode(ast_value);

		// search for duplicate attributes
		uint count = array_len(map->keys);
		for (uint i = 0; i < count; i++) {
			if(strcmp(attribute, map->keys[i]) == 0) {
				insert_idx = i;
				break;
			}
		}

		if(insert_idx == prop_idx) {
			array_append(map->keys, attribute);			
			array_append(map->values, value);
		} else {
			AR_EXP_Free(map->values[insert_idx]);
			map->values[insert_idx] = value;
		}
	}

	return map;
}

static PropertyMap *_PropertyMap_Clone
(
	PropertyMap *map
) {
	PropertyMap *clone = rm_malloc(sizeof(PropertyMap));
	array_clone(clone->keys, map->keys);
	array_clone_with_cb(clone->values, map->values, AR_EXP_Clone);

	return clone;
}

void PropertyMap_Free
(
	PropertyMap *map
) {
	if(map == NULL) return;

	array_free(map->keys);
	array_free_cb(map->values, AR_EXP_Free);
	rm_free(map);
}

NodeCreateCtx NodeCreateCtx_Clone
(
	NodeCreateCtx ctx
) {
	NodeCreateCtx clone = ctx;
	array_clone(clone.labels, ctx.labels);
	array_clone(clone.labelsId, ctx.labelsId);
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}

void NodeCreateCtx_Free
(
	NodeCreateCtx ctx
) {
	array_free(ctx.labels);
	array_free(ctx.labelsId);
	PropertyMap_Free(ctx.properties);
}

EdgeCreateCtx EdgeCreateCtx_Clone
(
	EdgeCreateCtx ctx
) {
	EdgeCreateCtx clone = ctx;
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}

EntityUpdateEvalCtx *UpdateCtx_New
(
	const char *alias
) {
	EntityUpdateEvalCtx *ctx = rm_malloc(sizeof(EntityUpdateEvalCtx));

	ctx->alias         = alias;
	ctx->record_idx    = INVALID_INDEX;
	ctx->properties    = array_new(PropertySetCtx, 1);
	ctx->add_labels    = NULL;
	ctx->remove_labels = NULL;

	return ctx;
}

EntityUpdateEvalCtx *UpdateCtx_Clone
(
	const EntityUpdateEvalCtx *orig
) {
	EntityUpdateEvalCtx *clone = rm_malloc(sizeof(EntityUpdateEvalCtx));

	uint count = array_len(orig->properties);

	clone->alias         = orig->alias;
	clone->record_idx    = orig->record_idx;
	clone->properties    = array_new(PropertySetCtx, count);
	clone->add_labels    = NULL;
	clone->remove_labels = NULL;
	if(orig->add_labels != NULL) {
		array_clone(clone->add_labels, orig->add_labels);
	}
	if(orig->remove_labels != NULL) {
		array_clone(clone->remove_labels, orig->remove_labels);
	}

	for(uint i = 0; i < count; i ++) {
		PropertySetCtx update = {
			.attribute = orig->properties[i].attribute,
			.exp = AR_EXP_Clone(orig->properties[i].exp),
			.mode = orig->properties[i].mode,
		};
		array_append(clone->properties, update);
	}

	return clone;
}

void UpdateCtx_Clear
(
	EntityUpdateEvalCtx *ctx
) {
	uint count = array_len(ctx->properties);
	for(uint i = 0; i < count; i ++) AR_EXP_Free(ctx->properties[i].exp);
	array_clear(ctx->properties);
}

void UpdateCtx_Free
(
	EntityUpdateEvalCtx *ctx
) {
	uint count = array_len(ctx->properties);
	for(uint i = 0; i < count; i ++) {
		AR_EXP_Free(ctx->properties[i].exp);
	}

	array_free(ctx->properties);
	if(ctx->add_labels != NULL) array_free(ctx->add_labels);
	if(ctx->remove_labels != NULL) array_free(ctx->remove_labels);

	rm_free(ctx);
}

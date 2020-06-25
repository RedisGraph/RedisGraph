#include "ast_shared.h"
#include "../util/rmalloc.h"
#include "ast_build_ar_exp.h"
#include <assert.h>

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
	assert(cypher_astnode_type(props) == CYPHER_AST_MAP); // TODO add parameter support

	uint prop_count = cypher_ast_map_nentries(props);

	PropertyMap *map = rm_malloc(sizeof(PropertyMap));
	map->keys = rm_malloc(prop_count * sizeof(Attribute_ID));
	map->values = rm_malloc(prop_count * sizeof(AR_ExpNode *));
	map->property_count = prop_count;

	for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
		const cypher_astnode_t *ast_key = cypher_ast_map_get_key(props, prop_idx);
		const char *attribute = cypher_ast_prop_name_get_value(ast_key);
		// Convert the string key to an Attribute ID.
		map->keys[prop_idx] = GraphContext_FindOrAddAttribute(gc, attribute);

		const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
		// Convert the AST entity representing the value into an expression to be resolved later.
		AR_ExpNode *value = AR_EXP_FromExpression(ast_value);
		map->values[prop_idx] = value;
	}
	return map;
}

static PropertyMap *_PropertyMap_Clone(PropertyMap *map) {
	PropertyMap *clone = rm_malloc(sizeof(PropertyMap));
	uint prop_count = map->property_count;
	clone->keys = rm_malloc(prop_count * sizeof(Attribute_ID));
	clone->values = rm_malloc(prop_count * sizeof(AR_ExpNode *));
	clone->property_count = prop_count;
	memcpy(clone->keys, map->keys, prop_count * sizeof(Attribute_ID));
	for(uint i = 0; i < prop_count; i++) clone->values[i] = AR_EXP_Clone(map->values[i]);

	return clone;
}

void PropertyMap_Free(PropertyMap *map) {
	if(map == NULL) return;

	for(uint i = 0; i < map->property_count; i++) {
		AR_EXP_Free(map->values[i]);
	}
	rm_free(map->keys);
	rm_free(map->values);
	rm_free(map);
}

EntityUpdateEvalCtx EntityUpdateEvalCtx_Clone(EntityUpdateEvalCtx ctx) {
	EntityUpdateEvalCtx clone = ctx;
	clone.exp = AR_EXP_Clone(ctx.exp);
	return clone;
}

NodeCreateCtx NodeCreateCtx_Clone(NodeCreateCtx ctx) {
	NodeCreateCtx clone = {0};
	clone.node = ctx.node;
	clone.node_idx = ctx.node_idx;
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}

EdgeCreateCtx EdgeCreateCtx_Clone(EdgeCreateCtx ctx) {
	EdgeCreateCtx clone = {0};
	clone.edge = ctx.edge;
	clone.src_idx = ctx.src_idx;
	clone.dest_idx = ctx.dest_idx;
	clone.edge_idx = ctx.edge_idx;
	if(ctx.properties) clone.properties = _PropertyMap_Clone(ctx.properties);
	return clone;
}


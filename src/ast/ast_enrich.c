#include "ast.h"

static char *_create_anon_alias(int anon_count) {
	char *alias;
	asprintf(&alias, "anon_%d", anon_count);
	return alias;
}

static void _name_anonymous_entities_in_pattern(const cypher_astnode_t *node,
												AnnotationCtx *annotation_ctx, uint *anon_count) {
	cypher_astnode_type_t t = cypher_astnode_type(node);
	const cypher_astnode_t *ast_identifier = NULL;
	if(t == CYPHER_AST_NODE_PATTERN) {
		ast_identifier = cypher_ast_node_pattern_get_identifier(node);
	} else if(t == CYPHER_AST_REL_PATTERN) {
		ast_identifier = cypher_ast_rel_pattern_get_identifier(node);
	} else {
		uint child_count = cypher_astnode_nchildren(node);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
			_name_anonymous_entities_in_pattern(child, annotation_ctx, anon_count);
		}
	}

	const char *alias;
	if(ast_identifier) {
		// Retrieve user-defined alias.
		alias = cypher_ast_identifier_get_name(ast_identifier);
	} else {
		// AST entity is an unaliased node, create an anonymous identifier.
		alias = _create_anon_alias((*anon_count)++);
	}

// Annotate AST entity with identifier string.
	cypher_astnode_attach_annotation(annotation_ctx, node, (void *)alias, NULL);
}

// AST annotation callback routine for freeing generated entity names only.
static void _Free_AnonCallback(void *userdata, const cypher_astnode_t *node, void *annotation) {
	if(!annotation) return;
	char *alias = annotation;
	if(!strncmp(alias, "anon_", 5)) free(alias); // TODO not ideal still!
}

// Construct a new annotation context for aliasing AST graph entities.
static AnnotationCtx *_AST_NewAnnotationCtx() {
	AnnotationCtx *annotation_ctx = cypher_ast_annotation_context();
	cypher_ast_annotation_context_release_handler_t handler = &_Free_AnonCallback;
	cypher_ast_annotation_context_set_release_handler(annotation_ctx, handler, NULL);
	return annotation_ctx;
}

void AST_Enrich(AST *ast) {
	/* Directives like CREATE INDEX are not queries. */
	if(cypher_astnode_type(ast->root) != CYPHER_AST_QUERY) return;

	ast->annotation_ctx = _AST_NewAnnotationCtx();

	uint anon_count = 0;
	_name_anonymous_entities_in_pattern(ast->root, ast->annotation_ctx, &anon_count);
}


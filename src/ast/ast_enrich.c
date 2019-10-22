#include "ast.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/rmalloc.h"
#include "../procedures/procedure.h"
#include <assert.h>

//------------------------------------------------------------------------------
//  Annotation context - graph entity naming
//------------------------------------------------------------------------------

// Compute the number of digits in a non-negative integer.
static inline int _digit_count(int n) {
	int count = 0;
	do {
		n /= 10;
		count++;
	} while(n);
	return count;
}

static inline char *_create_anon_alias(int anon_count) {
	// We need space for "anon_" (5), all digits, and a NULL terminator (1)
	int alias_len = _digit_count(anon_count) + 6;
	char *alias = rm_malloc(alias_len * sizeof(char));
	snprintf(alias, alias_len, "anon_%d", anon_count);
	return alias;
}

static void _annotate_entity_names(AST *ast, const cypher_astnode_t *node, uint *anon_count) {
	cypher_astnode_type_t t = cypher_astnode_type(node);
	const cypher_astnode_t *ast_identifier = NULL;
	// TODO write a toString function for CYPHER_AST_SORT_ITEM elements to populate them here
	// (currently added in ExecutionPlan construction)
	if(t == CYPHER_AST_NODE_PATTERN) {
		ast_identifier = cypher_ast_node_pattern_get_identifier(node);
	} else if(t == CYPHER_AST_REL_PATTERN) {
		ast_identifier = cypher_ast_rel_pattern_get_identifier(node);
	} else {
		// Did not encounter a graph entity, recursively visit children.
		uint child_count = cypher_astnode_nchildren(node);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
			_annotate_entity_names(ast, child, anon_count);
		}
		return; // Return to avoid annotating other AST nodes.
	}

	// The AST node is a graph entity.
	char *alias;
	if(ast_identifier) {
		// Graph entity has a user-defined alias, clone it for the annotation.
		alias = rm_strdup(cypher_ast_identifier_get_name(ast_identifier));
	} else {
		// Graph entity is an unaliased, create an anonymous identifier.
		alias = _create_anon_alias((*anon_count)++);
	}

	// Add AST annotation.
	AST_AttachName(ast, node, alias);
}

// AST annotation callback routine for freeing generated entity names.
static void _FreeNameAnnotationCallback(void *unused, const cypher_astnode_t *node,
										void *annotation) {
	rm_free(annotation);
}

// Construct a new annotation context for holding AST entity names (aliases or anonymous identifiers).
static AnnotationCtx *_AST_NewNameContext(void) {
	AnnotationCtx *name_ctx = cypher_ast_annotation_context();
	cypher_ast_annotation_context_release_handler_t handler = &_FreeNameAnnotationCallback;
	cypher_ast_annotation_context_set_release_handler(name_ctx, handler, NULL);
	return name_ctx;
}


//------------------------------------------------------------------------------
//  Annotation context - WITH/RETURN * projections
//------------------------------------------------------------------------------
static void _collect_aliases_in_path(const cypher_astnode_t *path, const char ***aliases) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Every even offset corresponds to a node.
	for(uint i = 0; i < path_len; i += 2) {
		const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(ast_node);
		if(ast_alias == NULL) continue;  // Unaliased node, do nothing.
		// Add node alias to projection array.
		*aliases = array_append(*aliases, cypher_ast_identifier_get_name(ast_alias));
	}

	// Every odd offset corresponds to an edge.
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *ast_edge = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias = cypher_ast_rel_pattern_get_identifier(ast_edge);
		if(ast_alias == NULL) continue;  // Unaliased edge, do nothing.
		// Add edge alias to projection array.
		*aliases = array_append(*aliases, cypher_ast_identifier_get_name(ast_alias));
	}
}

static void _collect_aliases_in_pattern(const cypher_astnode_t *pattern, const char ***aliases) {
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		_collect_aliases_in_path(cypher_ast_pattern_get_path(pattern, i), aliases);
	}
}

static void _collect_with_projections(const cypher_astnode_t *with_clause, const char ***aliases) {
	if(cypher_ast_with_has_include_existing(with_clause)) {
		const char **prev_aliases = AST_GetProjectAll(with_clause);
		*aliases = array_ensure_append(*aliases, prev_aliases, array_len(prev_aliases), const char *);
		return;
	}

	uint projection_count = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const char *alias = NULL;
		const cypher_astnode_t *identifier = cypher_ast_projection_get_alias(projection);
		if(identifier == NULL) {
			// The projection was not aliased, so the projection itself must be an identifier.
			identifier = cypher_ast_projection_get_expression(projection);
			assert(cypher_astnode_type(identifier) == CYPHER_AST_IDENTIFIER);
		}
		alias = cypher_ast_identifier_get_name(identifier);

		*aliases = array_append(*aliases, alias);
	}
}

// Sort an array of strings and remove duplicate entries.
static void _uniqueArray(const char **arr) {
#define MODIFIES_ISLT(a,b) (strcmp((*a),(*b)) < 0)
	int count = array_len(arr);
	if(count == 0) return;
	QSORT(const char *, arr, count, MODIFIES_ISLT);
	uint unique_idx = 0;
	for(int i = 0; i < count - 1; i ++) {
		if(strcmp(arr[i], arr[i + 1])) {
			arr[unique_idx++] = arr[i];
		}
	}
	arr[unique_idx++] = arr[count - 1];
	array_trimm_len(arr, unique_idx);
}

static void _collect_call_projections(const cypher_astnode_t *call_clause, const char ***aliases) {
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	if(yield_count == 0) {
		// If the procedure call is missing its yield part, include procedure outputs.
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		ProcedureCtx *proc = Proc_Get(proc_name);
		assert(proc);

		uint output_count = Procedure_OutputCount(proc);
		for(uint i = 0; i < output_count; i++) {
			const char *name = Procedure_GetOutput(proc, i);
			*aliases = array_append(*aliases, name);
		}
		Proc_Free(proc);
		return;
	}

	for(uint i = 0; i < yield_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node == NULL) alias_node = ast_exp;
		const char *identifier = cypher_ast_identifier_get_name(alias_node);

		*aliases = array_append(*aliases, identifier);
	}
}

static const char **_collect_aliases_in_scope(AST *ast, uint scope_start, uint scope_end) {
	assert(scope_start != scope_end);
	const char **aliases = array_new(const char *, 1);

	for(uint i = scope_start; i < scope_end; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_WITH) {
			// The WITH clause contains either aliases or its own STAR projection.
			_collect_with_projections(clause, &aliases);
		} else if(type == CYPHER_AST_MATCH) {
			// The MATCH clause contains one pattern of N paths.
			const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_CREATE) {
			// The CREATE clause contains one pattern of N paths.
			const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_MERGE) {
			// The MERGE clause contains one path.
			const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
			_collect_aliases_in_pattern(path, &aliases);
		} else if(type == CYPHER_AST_UNWIND) {
			// The UNWIND clause introduces one alias.
			const cypher_astnode_t *unwind_alias = cypher_ast_unwind_get_alias(clause);
			aliases = array_append(aliases, cypher_ast_identifier_get_name(unwind_alias));
		} else if(type == CYPHER_AST_CALL) {
			_collect_call_projections(clause, &aliases);
		}
	}

	// Sort and unique the aliases array so that we won't make redundant projections given queries like:
	// MATCH (a)-[]->(a) RETURN *
	_uniqueArray(aliases);

	return aliases;
}

static void _annotate_project_all(AST *ast) {
	uint *with_clause_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	uint with_clause_count = array_len(with_clause_indices);
	uint scope_start = 0;
	uint scope_end;
	for(uint i = 0; i < with_clause_count; i ++) {
		scope_end = with_clause_indices[i];
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, scope_end);
		if(cypher_ast_with_has_include_existing(clause)) {
			// Collect all aliases defined in this scope.
			const char **aliases = _collect_aliases_in_scope(ast, scope_start, scope_end);
			// Annotate the clause with the aliases array.
			cypher_astnode_attach_annotation(ast->project_all_ctx, clause, (void *)aliases, NULL);
		}
		scope_start = scope_end;
	}
	array_free(with_clause_indices);

	uint last_clause_index = cypher_ast_query_nclauses(ast->root) - 1;
	const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, last_clause_index);
	if(cypher_astnode_type(last_clause) == CYPHER_AST_RETURN &&
	   cypher_ast_return_has_include_existing(last_clause)) {
		// Collect all aliases defined in this scope.
		const char **aliases = _collect_aliases_in_scope(ast, scope_start, last_clause_index);
		// Annotate the clause with the aliases array.
		cypher_astnode_attach_annotation(ast->project_all_ctx, last_clause, (void *)aliases, NULL);
	}
}

// AST annotation callback routine for freeing projection arrays.
static void _FreeProjectAllAnnotationCallback(void *unused, const cypher_astnode_t *node,
											  void *annotation) {
	array_free(annotation);
}

// Construct a new annotation context for holding names in WITH/RETURN * projections.
static AnnotationCtx *_AST_NewProjectAllContext(void) {
	AnnotationCtx *project_all_ctx = cypher_ast_annotation_context();
	cypher_ast_annotation_context_release_handler_t handler = &_FreeProjectAllAnnotationCallback;
	cypher_ast_annotation_context_set_release_handler(project_all_ctx, handler, NULL);
	return project_all_ctx;
}

// TODO this function is exposed since ORDER BY expressions are named later - possibly can be refactored.
void AST_AttachName(AST *ast, const cypher_astnode_t *node, const char *name) {
	// Annotate AST entity with identifier string.
	cypher_astnode_attach_annotation(ast->name_ctx, node, (void *)name, NULL);
}

void AST_Enrich(AST *ast) {
	/* Directives like CREATE INDEX are not queries. */
	if(cypher_astnode_type(ast->root) != CYPHER_AST_QUERY) return;

	// Instantiate an annotation context for accessing AST entity names.
	ast->name_ctx = _AST_NewNameContext();

	uint anon_count = 0;
	// Generate all name annotations.
	_annotate_entity_names(ast, ast->root, &anon_count);

	// Instantiate an annotation context for augmenting STAR projections.
	ast->project_all_ctx = _AST_NewProjectAllContext();
	// Generate annotations for WITH/RETURN * clauses.
	_annotate_project_all(ast);
}


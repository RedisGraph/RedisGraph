/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "annotate_pattern_comprehension.h"
#include "../../util/rmalloc.h"


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
	// We need space for "anon_pc_" (8), all digits, and a NULL terminator (1)
	int alias_len = _digit_count(anon_count) + 9;
	char *alias = rm_malloc(alias_len * sizeof(char));
	snprintf(alias, alias_len, "anon_pc_%d", anon_count);
	return alias;
}

static void _annotate_pattern_comprehension(AST *ast, const cypher_astnode_t *node, uint *anon_count) {
	cypher_astnode_type_t t = cypher_astnode_type(node);
	const cypher_astnode_t *ast_identifier = NULL;

	if(t == CYPHER_AST_PATTERN_COMPREHENSION || t == CYPHER_AST_PATTERN_PATH) {
        // The AST node is a graph entity.
        char *alias = _create_anon_alias((*anon_count)++);
        // Add AST annotation.
        AST_AttachName(ast, node, alias);
	}

    uint child_count = cypher_astnode_nchildren(node);
    for(uint i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        _annotate_pattern_comprehension(ast, child, anon_count);
    }
}

void AST_AnnotatePatternComprehension(AST *ast) {
	uint anon_count = 0;
	// Generate all name annotations.
	_annotate_pattern_comprehension(ast, ast->root, &anon_count);
}

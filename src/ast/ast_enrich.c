/*
* Copyright 2018 - 2022 Redis Labs Ltd. and Contributors
*
*This file is available under the Redis Labs Source Available License Agreement
*/

#include "enrichment/annotate_projected_named_paths.h"

//------------------------------------------------------------------------------
//  Main AST enrichment
//------------------------------------------------------------------------------

void AST_Enrich(AST *ast) {
	/* Directives like CREATE INDEX are not queries. */
	if(cypher_astnode_type(ast->root) != CYPHER_AST_QUERY) return;
	AST_AnnotateNamedPaths(ast);
}

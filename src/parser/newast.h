/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef NEW_AST_H
#define NEW_AST_H

#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

// Checks if AST represent a read only query.
bool NEWAST_ReadOnly(const cypher_parse_result_t *ast);

// Checks to see if AST contains specified clause. 
bool NEWAST_ContainsClause(const cypher_parse_result_t *ast, const char* clause);

// Checks to see if query contains any errors.
bool NEWAST_ContainsErrors(const cypher_parse_result_t *ast);

// Returns specified clause of NULL.
const cypher_astnode_t *NEWAST_GetClause(const cypher_parse_result_t *ast, const char* clause);

// Report encountered errors.
char* NEWAST_ReportErrors(const cypher_parse_result_t *ast);

// Checks if RETURN clause contains collapsed entities.
int NEWAST_ReturnClause_ContainsCollapsedNodes(const cypher_parse_result_t *ast);

#endif

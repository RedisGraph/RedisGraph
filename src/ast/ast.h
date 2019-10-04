/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../value.h"
#include "cypher-parser.h"
#include "../redismodule.h"
#include "rax.h"

typedef const void *AST_IDENTIFIER;

#define IDENTIFIER_NOT_FOUND UINT_MAX

typedef enum {
	AST_VALID,
	AST_INVALID
} AST_Validation;

typedef struct {
	const cypher_astnode_t *root;     // Root element of libcypher-parser AST
	rax *referenced_entities;         // Mapping of the referenced entities.
	bool free_root;                   // The root should only be freed if this is a sub-AST we constructed
} AST;

// Checks to see if libcypher-parser reported any errors.
bool AST_ContainsErrors(const cypher_parse_result_t *result);

// Make sure the parse result and the AST tree pass all validations.
AST_Validation AST_Validate(RedisModuleCtx *ctx, const cypher_parse_result_t *result);

// Checks if the parse result represents a read-only query.
bool AST_ReadOnly(const cypher_parse_result_t *result);

// Checks to see if AST contains specified clause.
bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause);

// Checks if the AST contains a RETURN clause.
bool AST_ContainsReturn(const AST *ast);

// Returns all function (aggregated & none aggregated) mentioned in query.
void AST_ReferredFunctions(const cypher_astnode_t *root, rax *referred_funcs);

// Returns specified clause or NULL.
const cypher_astnode_t *AST_GetClause(const AST *ast, cypher_astnode_type_t clause_type);

// Returns the indexes into the AST of all instances of the given clause.
uint *AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type);

// Returns the number of times the given clause appears in the AST.
uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type);

// Returns all instances of the given clause in the AST.
const cypher_astnode_t **AST_GetClauses(const AST *ast, cypher_astnode_type_t type);

// Collect all user-provided aliases in the query.
// (Only used for populating return expressions when "RETURN *" is specified.)
const char **AST_CollectElementNames(AST *ast);

AST *AST_Build(cypher_parse_result_t *parse_result);

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset);

// Returns true if the given alias is referenced within this AST segment.
bool AST_AliasIsReferenced(AST *ast, const char *alias);

// Convert an AST integer node (which is stored internally as a string) into an integer.
long AST_ParseIntegerNode(const cypher_astnode_t *int_node);

// Returns true if the given clause contains an aggregate function.
bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause);

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast);

void AST_Free(AST *ast);


/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../value.h"
#include "cypher-parser.h"
#include "../redismodule.h"
#include "rax.h"
#include "ast_annotations_ctx_collection.h"
#include "../arithmetic/arithmetic_expression.h"

#define IDENTIFIER_NOT_FOUND UINT_MAX
#define UNLIMITED UINT_MAX

typedef enum {
	AST_VALID,
	AST_INVALID
} AST_Validation;

typedef struct {
	const cypher_astnode_t *root;                       // Root element of libcypher-parser AST
	rax *referenced_entities;                           // Mapping of the referenced entities.
	AST_AnnotationCtxCollection *anot_ctx_collection;   // Holds annotations contexts.
	rax *canonical_entity_names;                        // Storage for canonical graph entity names.
	AR_ExpNode *limit;                                  // The number of results in this segment.
	AR_ExpNode *skip;                                   // The number of skips in this segment.
	bool free_root;                                     // The root should only be freed if this is a sub-AST we constructed
	uint ref_count;                                     // Reference counter for deletion.
	cypher_parse_result_t *parse_result;                // Query parsing output.
	cypher_parse_result_t *params_parse_result;         // Parameters parsing output.
} AST;

// Checks to see if libcypher-parser reported any errors.
bool AST_ContainsErrors(const cypher_parse_result_t *result);

// Make sure the parse result and the AST tree pass all validations.
AST_Validation AST_Validate_Query(const cypher_parse_result_t *result);

// Validate query parameters parsing only.
AST_Validation AST_Validate_QueryParams(const cypher_parse_result_t *result);

// Checks if the parse result represents a read-only query.
bool AST_ReadOnly(const cypher_astnode_t *root);

// Checks to see if AST contains specified clause.
bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause);

// Checks to see if an AST tree contains specified node type.
bool AST_TreeContainsType(const cypher_astnode_t *root, cypher_astnode_type_t clause);

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

// Returns an array (arr.h type) of all the nodes from a given type. Note: array must be free after use.
const cypher_astnode_t **AST_GetTypedNodes(const cypher_astnode_t *ast,
										   cypher_astnode_type_t type);

// Collect all aliases within `entity` subtree.
void AST_CollectAliases(const char ***aliases, const cypher_astnode_t *entity);

AST *AST_Build(cypher_parse_result_t *parse_result);

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset);

// Sets a parameter parsing result in the ast.
void AST_SetParamsParseResult(AST *ast, cypher_parse_result_t *params_parse_result);

// Returns a shallow copy of the original AST pointer with ref counter increased.
AST *AST_ShallowCopy(AST *orig);

// Populate the AST's map of all referenced aliases.
void AST_BuildReferenceMap(AST *ast, const cypher_astnode_t *project_clause);

// Annotate AST, naming all anonymous graph entities.
void AST_Enrich(AST *ast);

// Annotate the AST node with the given name.
void AST_AttachName(AST *ast, const cypher_astnode_t *node, const char *name);

// Returns true if the given alias is referenced within this AST segment.
bool AST_AliasIsReferenced(AST *ast, const char *alias);

// Convert an AST integer node (which is stored internally as a string) into an integer.
long AST_ParseIntegerNode(const cypher_astnode_t *int_node);

// Returns true if the given clause contains an aggregate function.
bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause);

// Retrieve the name of a given AST entity. This interface is valid for
// AST node and edge patterns as well as any ORDER BY item.
const char *AST_GetEntityName(const AST *ast, const cypher_astnode_t *entity);

// Retrieve the array of projected aliases for a WITH/RETURN * clause.
const char **AST_GetProjectAll(const cypher_astnode_t *projection_clause);

// Collect the aliases from a RETURN clause to populate ResultSet column names.
const char **AST_BuildReturnColumnNames(const cypher_astnode_t *return_clause);

// Collect the aliases from a CALL clause to populate ResultSet column names.
const char **AST_BuildCallColumnNames(const cypher_astnode_t *return_clause);

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast);

// Parse a query to construct an immutable AST.
cypher_parse_result_t *parse_query(const char *query);

// Parse a query parameter values only. The remaining query string is set in the result body.
cypher_parse_result_t *parse_params(const char *query, const char **query_body);

// Free the immutable AST generated by the parser.
void parse_result_free(cypher_parse_result_t *parse_result);

// Returns the ast annotation context collection of the AST.
AST_AnnotationCtxCollection *AST_GetAnnotationCtxCollection(AST *ast);

AR_ExpNode *AST_GetLimitExpr(const AST *ast);

uint64_t AST_GetLimit(const AST *ast);

AR_ExpNode *AST_GetSkipExpr(const AST *ast);

uint64_t AST_GetSkip(const AST *ast);

void AST_Free(AST *ast);


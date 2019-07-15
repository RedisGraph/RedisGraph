/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// #include "/ast_shared.h"
#include "../value.h"
#include "../redismodule.h"
#include "../util/triemap/triemap.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

typedef const void* AST_IDENTIFIER;

#define IDENTIFIER_NOT_FOUND UINT_MAX

typedef enum {
    AST_VALID,
    AST_INVALID
} AST_Validation;

typedef struct {
    const cypher_astnode_t *root;     // Root element of libcypher-parser AST
    TrieMap *entity_map;              // Mapping of aliases and AST node pointers to AST IDs
} AST;

// AST clause validations.
AST_Validation AST_Validate(RedisModuleCtx *ctx, const AST *ast);

// Checks if AST represents a read-only query.
bool AST_ReadOnly(const cypher_astnode_t *root);

// Checks to see if AST contains specified clause. 
bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause);

// Checks to see if query contains any errors.
bool AST_ContainsErrors(const cypher_parse_result_t *result);

// Report encountered errors.
char* AST_ReportErrors(const cypher_parse_result_t *result);

// Returns all function (aggregated & none aggregated) mentioned in query.
void AST_ReferredFunctions(const cypher_astnode_t *root, TrieMap *referred_funcs);

// Returns specified clause or NULL.
const cypher_astnode_t* AST_GetClause(const AST *ast, cypher_astnode_type_t clause_type);

uint* AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type);

uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type);

const cypher_astnode_t** AST_GetClauses(const AST *ast, cypher_astnode_type_t type);

const char** AST_CollectAliases(AST *ast);

const cypher_astnode_t* AST_GetBody(const cypher_parse_result_t *result);

AST* AST_Build(cypher_parse_result_t *parse_result);

AST* AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset);

long AST_ParseIntegerNode(const cypher_astnode_t *int_node);

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause);

/* Given an AST entity representing a node, edge, identifier, or property specification,
 * return its alias (if one is provided). */
const char* AST_GetEntityAlias(const cypher_astnode_t *entity);

// mapping functions
uint AST_GetEntityIDFromReference(const AST *ast, AST_IDENTIFIER entity);

uint AST_GetEntityIDFromAlias(const AST *ast, const char *alias);

uint ASTMap_AddEntity(const AST *ast, AST_IDENTIFIER identifier, uint id);

uint ASTMap_FindOrAddAlias(const AST *ast, const char *alias, uint id);

void AST_BuildEntityMap(AST *ast);

AST* AST_GetFromTLS(void);

void AST_Free(AST *ast);

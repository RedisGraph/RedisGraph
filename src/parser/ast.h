/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#ifndef NEW_AST_H
#define NEW_AST_H

// #include "/ast_shared.h"
#include "../value.h"
#include "../util/triemap/triemap.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

typedef const cypher_astnode_t* AST_IDENTIFIER;

#define NOT_IN_RECORD UINT_MAX

// #define IDENTIFIER_NOT_FOUND UINT_MAX

typedef enum {
    AST_VALID,
    AST_INVALID
} AST_Validation;

typedef struct AR_ExpNode AR_ExpNode;

typedef struct {
    const cypher_astnode_t *root;
    // Extensible array of entities described in MATCH, MERGE, and CREATE clauses
    AR_ExpNode **defined_entities;
    TrieMap *entity_map;
    unsigned int record_length; // TODO can be moved to segment?
    unsigned int start_offset;    // Left-hand bound of AST clauses to consider
    unsigned int end_offset;    // Right-hand bound of AST clauses to consider
} AST;

// AST clause validations.
AST_Validation AST_Validate(const AST *ast, char **reason);

// Checks if AST represent a read only query.
bool AST_ReadOnly(const AST *query);

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

unsigned int AST_GetTopLevelClauses(const AST *ast, cypher_astnode_type_t clause_type, const cypher_astnode_t **matches);

uint* AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type);

uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type);

uint AST_NumClauses(const AST *ast);

const cypher_astnode_t** AST_CollectReferencesInRange(const AST *ast, cypher_astnode_type_t type);

const cypher_astnode_t* AST_GetBody(const cypher_parse_result_t *result);

AST* AST_Build(cypher_parse_result_t *parse_result);

long AST_ParseIntegerNode(const cypher_astnode_t *int_node);

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause);

AR_ExpNode** AST_GetOrderExpressions(const cypher_astnode_t *order_clause);

void AST_BuildAliasMap(AST *ast);

// mapping functions

unsigned int AST_GetAliasID(const AST *ast, char *alias);

void AST_MapEntity(const AST *ast, AST_IDENTIFIER identifier, AR_ExpNode *exp);

void AST_MapAlias(const AST *ast, char *alias, AR_ExpNode *exp);

AR_ExpNode* AST_GetEntity(const AST *ast, AST_IDENTIFIER entity);

AR_ExpNode* AST_GetEntityFromAlias(const AST *ast, char *alias);

unsigned int AST_GetEntityRecordIdx(const AST *ast, const cypher_astnode_t *entity);

// TODO find better place for record code
unsigned int AST_RecordLength(const AST *ast);

unsigned int AST_AddRecordEntry(AST *ast);

void AST_RecordAccommodateExpression(AST *ast, AR_ExpNode *exp);

unsigned int AST_AddAnonymousRecordEntry(AST *ast);

AST* AST_GetFromTLS(void);

#endif

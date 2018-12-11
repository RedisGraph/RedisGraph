/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "../value.h"
#include "./ast_common.h"
#include "../redismodule.h"
#include "../util/vector.h"
#include "./clauses/clauses.h"

typedef enum {
	AST_VALID,
	AST_INVALID
} AST_Validation;

typedef struct {
	AST_MatchNode *matchNode;
	AST_CreateNode *createNode;
	AST_MergeNode *mergeNode;
	AST_SetNode *setNode;
	AST_DeleteNode *deleteNode;
	AST_WhereNode *whereNode;
	AST_ReturnNode *returnNode;
	AST_OrderNode *orderNode;
	AST_LimitNode *limitNode;
	AST_SkipNode *skipNode;
	AST_IndexNode *indexNode;
	AST_UnwindNode *unwindNode;
	TrieMap *_aliasIDMapping;	// Mapping between aliases and IDs.
} AST;

AST* AST_New(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
						 AST_CreateNode *createNode, AST_MergeNode *mergeNode,
						 AST_SetNode *setNode, AST_DeleteNode *deleteNode,
						 AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
						 AST_SkipNode *skipNode, AST_LimitNode *limitNode,
						 AST_IndexNode *indexNode, AST_UnwindNode *unwindNode);

// Retrieve AST from thread local storage.
AST *AST_GetFromLTS();

// AST clause validations.
AST_Validation AST_Validate(const AST* ast, char **reason);

// Returns number of aliases defined in AST.
int AST_AliasCount(const AST *ast);

// Returns alias ID.
int AST_GetAliasID(const AST *ast, char *alias);

void AST_NameAnonymousNodes(AST *ast);

// Checks if AST represent a read only query.
bool AST_ReadOnly(const AST *ast);

void AST_Free(AST *queryExpressionNode);

#endif

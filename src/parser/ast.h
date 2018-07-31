/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef AST_H
#define AST_H

#include "../value.h"
#include "./ast_common.h"
#include "../redismodule.h"
#include "../rmutil/vector.h"
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
	AST_IndexNode *indexNode;
} AST_Query;

AST_Query* New_AST_Query(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
						 AST_CreateNode *createNode, AST_MergeNode *mergeNode,
						 AST_SetNode *setNode, AST_DeleteNode *deleteNode,
						 AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
						 AST_LimitNode *limitNode, AST_IndexNode *indexNode);

/* AST clause validations. */
AST_Validation _Validate_CREATE_Clause(const AST_Query* ast, char **reason);
AST_Validation _Validate_DELETE_Clause(const AST_Query* ast, char **reason);
AST_Validation _Validate_MATCH_Clause(const AST_Query* ast, char **reason);
AST_Validation _Validate_RETURN_Clause(const AST_Query* ast, char **reason);
AST_Validation _Validate_SET_Clause(const AST_Query* ast, char **reason);
AST_Validation _Validate_WHERE_Clause(const AST_Query* ast, char **reason);
AST_Validation AST_Validate(const AST_Query* ast, char **reason);

void AST_NameAnonymousNodes(AST_Query *ast);

void Free_AST_Query(AST_Query *queryExpressionNode);

#endif

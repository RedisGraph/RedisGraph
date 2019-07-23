/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../util/triemap/triemap.h"

// Procedure call clause describes an invocation of a procedure.
typedef struct {
	char *procedure;        // Procedure name to call
	char **arguments;       // List of arguments to pass
	char **yield;           // Procedure output(s).
} AST_ProcedureCallNode;

AST_ProcedureCallNode *New_AST_ProcedureCallNode(char *procedure,
												 char **arguments, char **yield);

/* Lists entities consumed by this clause. */
void ProcedureCallClause_ReferredEntities(const AST_ProcedureCallNode *node,
										  TrieMap *referred_entities);

/* Lists entities defined by this clause. */
void ProcedureCallClause_DefinedEntities(const AST_ProcedureCallNode *node,
										 TrieMap *definedEntities);

void Free_AST_ProcedureCallNode(AST_ProcedureCallNode *node);

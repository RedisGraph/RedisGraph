/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _UNWIND_H_
#define _UNWIND_H_

#include "../../util/vector.h"
#include "../../util/triemap/triemap.h"

// Unwind clause describes a list of arithmetic expressions
typedef struct {    
    Vector *expressions;    // Vector of AST_ArithmeticExpressionNode pointers.
    char *alias;
} AST_UnwindNode;

AST_UnwindNode* New_AST_UnwindNode(Vector *expressions, char *alias);

/* Adds alias to referred_entities. */
void UnwindClause_ReferredEntities(const AST_UnwindNode *unwindNode, TrieMap *referred_entities);

void Free_AST_UnwindNode(AST_UnwindNode *unwindNode);

#endif

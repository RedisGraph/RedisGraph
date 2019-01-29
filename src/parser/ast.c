/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../util/rmalloc.h"
#include "../graph/entities/graph_entity.h"
#include "./ast_arithmetic_expression.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

static void _AST_MapAliasToID(AST *ast) {
  uint id = 0;  
  ast->_aliasIDMapping = NewTrieMap(); // Holds mapping between referred entities and IDs.
  
  // Get unique aliases, from clauses which can introduce entities.
  TrieMap *referredEntities = NewTrieMap();
  MatchClause_DefinedEntities(ast->matchNode, referredEntities);
  CreateClause_ReferredEntities(ast->createNode, referredEntities);
  UnwindClause_DefinedEntities(ast->unwindNode, referredEntities);
  ReturnClause_DefinedEntities(ast->returnNode, referredEntities);

  void *val;
  char *ptr;
  tm_len_t len;
  TrieMapIterator *it = TrieMap_Iterate(referredEntities, "", 0);
  // Scan through aliases and give each one an ID.
  while(TrieMapIterator_Next(it, &ptr, &len, &val)) {
    uint *entityID = malloc(sizeof(uint));
    *entityID = id++;
    TrieMap_Add(ast->_aliasIDMapping, ptr, len, entityID, TrieMap_NOP_REPLACE);
  }

  TrieMapIterator_Free(it);
  TrieMap_Free(referredEntities, TrieMap_NOP_CB);
}

AST *AST_New(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
                         AST_CreateNode *createNode, AST_MergeNode *mergeNode,
                         AST_SetNode *setNode, AST_DeleteNode *deleteNode,
                         AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
                         AST_SkipNode *skipNode, AST_LimitNode *limitNode,
                         AST_IndexNode *indexNode, AST_UnwindNode *unwindNode) {
  AST *ast = rm_malloc(sizeof(AST));

  ast->matchNode = matchNode;
  ast->whereNode = whereNode;
  ast->createNode = createNode;
  ast->mergeNode = mergeNode;
  ast->setNode = setNode;
  ast->deleteNode = deleteNode;
  ast->returnNode = returnNode;
  ast->orderNode = orderNode;
  ast->skipNode = skipNode;
  ast->limitNode = limitNode;
  ast->indexNode = indexNode;
  ast->unwindNode = unwindNode;
  ast->_aliasIDMapping = NULL;
  return ast;
}

AST *AST_GetFromLTS() {
  AST* ast = pthread_getspecific(_tlsASTKey);
  assert(ast);
  return ast;
}

int AST_AliasCount(const AST *ast) {
  assert(ast);
  return ast->_aliasIDMapping->cardinality;
}

int AST_GetAliasID(const AST *ast, char *alias) {
  assert(ast->_aliasIDMapping);
  void *v = TrieMap_Find(ast->_aliasIDMapping, alias, strlen(alias));
  assert(v != TRIEMAP_NOTFOUND);
  int *id = (int*)v;
  return *id;
}

void AST_NameAnonymousNodes(AST *ast) {
  int entity_id = 0;

  if(ast->matchNode)
    MatchClause_NameAnonymousNodes(ast->matchNode, &entity_id);

  if(ast->createNode)
    CreateClause_NameAnonymousNodes(ast->createNode, &entity_id);
  
  if(ast->mergeNode)
    MergeClause_NameAnonymousNodes(ast->mergeNode, &entity_id);
  
  // Mark each alias with a unique ID.
  _AST_MapAliasToID(ast);
}

bool AST_ReadOnly(const AST *ast) {
  return !(ast->createNode != NULL ||
           ast->mergeNode != NULL ||
           ast->deleteNode != NULL ||
           ast->setNode != NULL ||
           ast->indexNode != NULL);
}

void AST_Free(AST *ast) {
  Free_AST_MatchNode(ast->matchNode);
  Free_AST_CreateNode(ast->createNode);
  Free_AST_MergeNode(ast->mergeNode);
  Free_AST_DeleteNode(ast->deleteNode);
  Free_AST_SetNode(ast->setNode);
  Free_AST_WhereNode(ast->whereNode);
  Free_AST_ReturnNode(ast->returnNode);
  Free_AST_SkipNode(ast->skipNode);
  Free_AST_OrderNode(ast->orderNode);
  Free_AST_UnwindNode(ast->unwindNode);

  if(ast->_aliasIDMapping) TrieMap_Free(ast->_aliasIDMapping, NULL);
  rm_free(ast);
}

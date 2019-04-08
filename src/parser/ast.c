/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../util/arr.h"
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
    TrieMap_Add(ast->_aliasIDMapping, ptr, len, entityID, TrieMap_DONT_CARE_REPLACE);
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
  ast->withNode = NULL;
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
}

void AST_MapAliasToID(AST *ast, AST_WithNode *prevWithClause) {
  uint id = 0;
  uint *entityID = NULL;
  ast->_aliasIDMapping = NewTrieMap(); // Holds mapping between referred entities and IDs.
  
  if(prevWithClause) {
    for(uint i = 0; i < array_len(prevWithClause->exps); i++) {
      entityID = malloc(sizeof(uint));
      *entityID = id++;
      AST_WithElementNode *exp = prevWithClause->exps[i];
      TrieMap_Add(ast->_aliasIDMapping,
                  exp->alias,
                  strlen(exp->alias),
                  entityID,
                  TrieMap_DONT_CARE_REPLACE);
    }
  }

  // Get unique aliases, from clauses which can introduce entities.
  TrieMap *definedEntities = NewTrieMap();
  MatchClause_DefinedEntities(ast->matchNode, definedEntities);
  CreateClause_DefinedEntities(ast->createNode, definedEntities);
  UnwindClause_DefinedEntities(ast->unwindNode, definedEntities);
  ReturnClause_DefinedEntities(ast->returnNode, definedEntities);
  WithClause_DefinedEntities(ast->withNode, definedEntities);

  void *val;
  char *ptr;
  tm_len_t len;
  TrieMapIterator *it = TrieMap_Iterate(definedEntities, "", 0);
  // Scan through aliases and give each one an ID.
  while(TrieMapIterator_Next(it, &ptr, &len, &val)) {
    entityID = malloc(sizeof(uint));
    *entityID = id++;
    TrieMap_Add(ast->_aliasIDMapping, ptr, len, entityID, TrieMap_DONT_CARE_REPLACE);
  }

  TrieMapIterator_Free(it);
  TrieMap_Free(definedEntities, TrieMap_NOP_CB);
}

// Returns a triemap of all identifiers defined by ast.
TrieMap* AST_Identifiers(const AST *ast) {
  TrieMap *identifiers = NewTrieMap();
  MatchClause_DefinedEntities(ast->matchNode, identifiers);
  ReturnClause_DefinedEntities(ast->returnNode, identifiers);
  WithClause_DefinedEntities(ast->withNode, identifiers);
  CreateClause_DefinedEntities(ast->createNode, identifiers);
  UnwindClause_DefinedEntities(ast->unwindNode, identifiers);
  return identifiers;
}

bool AST_ReadOnly(AST **ast) {
  for (uint i = 0; i < array_len(ast); i++) {
    bool write = (ast[i]->createNode ||
                  ast[i]->mergeNode ||
                  ast[i]->deleteNode ||
                  ast[i]->setNode ||
                  ast[i]->indexNode);
    if(write) return false;
  }
  return true;
}

void AST_Free(AST **ast) {
  for (uint i = 0; i < array_len(ast); i++) {
    Free_AST_MatchNode(ast[i]->matchNode);
    Free_AST_CreateNode(ast[i]->createNode);
    Free_AST_MergeNode(ast[i]->mergeNode);
    Free_AST_DeleteNode(ast[i]->deleteNode);
    Free_AST_SetNode(ast[i]->setNode);
    Free_AST_WhereNode(ast[i]->whereNode);
    Free_AST_ReturnNode(ast[i]->returnNode);
    Free_AST_SkipNode(ast[i]->skipNode);
    Free_AST_OrderNode(ast[i]->orderNode);
    Free_AST_UnwindNode(ast[i]->unwindNode);
    Free_AST_LimitNode(ast[i]->limitNode);
    Free_AST_WithNode(ast[i]->withNode);

    if(ast[i]->_aliasIDMapping) TrieMap_Free(ast[i]->_aliasIDMapping, NULL);
    rm_free(ast[i]);
  }
  array_free(ast);
}

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
#include "../graph/entities/graph_entity.h"
#include "./ast_arithmetic_expression.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

/* Compares a triemap of user-specified functions with the registered functions we provide. */
AST_Validation _AST_ValidateReferredFunctions(TrieMap *referred_functions,
                                              char **reason,
                                              bool include_aggregates) {
  void *value;
  tm_len_t len;
  char *funcName;
  TrieMapIterator *it = TrieMap_Iterate(referred_functions, "", 0);
  AST_Validation res = AST_VALID;
  *reason = NULL;

  // TODO: return RAX.
  while(TrieMapIterator_Next(it, &funcName, &len, &value)) {
    funcName[len] = 0;
    // No functions have a name longer than 32 characters
    if (len >= 32) {
      res = AST_INVALID;
      break;
    }

    if (AR_FuncExists(funcName)) continue;

    if (Agg_FuncExists(funcName)) {
      if (include_aggregates) {
        continue;
      } else {
        // Provide a unique error for using aggregate functions from inappropriate contexts
        asprintf(reason, "Invalid use of aggregating function '%s'", funcName);
        res = AST_INVALID;
        break;
      }
    }

    // If we reach this point, the function was not found
    res = AST_INVALID;
    break;
  }

  // If the function was not found, provide a reason if one is not set
  if (res == AST_INVALID && *reason == NULL) {
    asprintf(reason, "Unknown function '%s'", funcName);
  }

  TrieMapIterator_Free(it);

  return res;
}

AST_Query *New_AST_Query(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
                         AST_CreateNode *createNode, AST_MergeNode *mergeNode,
                         AST_SetNode *setNode, AST_DeleteNode *deleteNode,
                         AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
                         AST_SkipNode *skipNode, AST_LimitNode *limitNode,
                         AST_IndexNode *indexNode) {
  AST_Query *queryExpressionNode = (AST_Query *)malloc(sizeof(AST_Query));

  queryExpressionNode->matchNode = matchNode;
  queryExpressionNode->whereNode = whereNode;
  queryExpressionNode->createNode = createNode;
  queryExpressionNode->mergeNode = mergeNode;
  queryExpressionNode->setNode = setNode;
  queryExpressionNode->deleteNode = deleteNode;
  queryExpressionNode->returnNode = returnNode;
  queryExpressionNode->orderNode = orderNode;
  queryExpressionNode->skipNode = skipNode;
  queryExpressionNode->limitNode = limitNode;
  queryExpressionNode->indexNode = indexNode;

  return queryExpressionNode;
}

AST_Validation _Validate_Aliases_In_Match_Clause(TrieMap *aliasesToCheck,
                                                const AST_MatchNode *match_node,
                                                char **undefined_alias) {
  /* Check that all the aliases that are in aliasesToCheck exists in the match clause */
  AST_Validation res = AST_VALID;
  TrieMap *matchAliases = NewTrieMap();
  MatchClause_ReferredEntities(match_node, matchAliases);

  TrieMapIterator *it = TrieMap_Iterate(aliasesToCheck, "", 0);
  char *ptr;
  tm_len_t len;
  void *value;

  while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
    if(TrieMap_Find(matchAliases, ptr, len) == TRIEMAP_NOTFOUND) {
      asprintf(undefined_alias, "%s not defined", ptr);
      res = AST_INVALID;
      break;
    }
  }

  TrieMapIterator_Free(it);
  TrieMap_Free(matchAliases, TrieMap_NOP_CB);
  return res;
}

AST_Validation _Validate_CREATE_Clause(const AST_Query *ast, char **reason) {
  if(!ast->createNode) return AST_VALID;
  AST_CreateNode *createNode = ast->createNode;
  
  size_t entityCount = Vector_Size(createNode->graphEntities);
  for(int i = 0; i < entityCount; i++) {
    AST_GraphEntity *entity;
    Vector_Get(createNode->graphEntities, i, &entity);
    
    if(entity->t == N_ENTITY) continue;
    if (!entity->label) {
      asprintf(reason, "Exactly one relationship type must be specified for CREATE");
      return AST_INVALID;
    }
  }

  return AST_VALID;
}

AST_Validation _Validate_DELETE_Clause(const AST_Query *ast, char **reason) {

  if (!ast->deleteNode) {
    return AST_VALID;
  }

  if (!ast->matchNode) {
    return AST_INVALID;
  }

  TrieMap *delete_aliases = NewTrieMap();
  DeleteClause_ReferredEntities(ast->deleteNode, delete_aliases);

  AST_Validation res = _Validate_Aliases_In_Match_Clause(delete_aliases, ast->matchNode, reason);
  TrieMap_Free(delete_aliases, TrieMap_NOP_CB);
  return res;
}

AST_Validation _Validate_MATCH_Clause(const AST_Query *ast, char **reason) {
  if(!ast->matchNode) return AST_VALID;
  
  // Check to see if an edge is being reused.
  AST_Validation res = AST_VALID;
  TrieMap *edgeAliases = NewTrieMap();  // Will let us know if duplicates.

  // Scan mentioned edges.
  size_t entityCount = Vector_Size(ast->matchNode->_mergedPatterns);
  for(int i = 0; i < entityCount; i++) {
    AST_GraphEntity *entity;
    Vector_Get(ast->matchNode->_mergedPatterns, i, &entity);

    if(entity->t != N_LINK) continue;

    AST_LinkEntity *edge = (AST_LinkEntity*) entity;
    if(edge->length) {
      if(edge->length->minHops > edge->length->maxHops) {
        asprintf(reason, "Variable length path, maximum number of hops must be greater or equal to minimum number of hops.");
        res = AST_INVALID;
        break;
      }
    }

    char *alias = entity->alias;
    /* The query is validated before and after aliasing anonymous entities,
     * so alias may be NULL at this time. */
    if (!alias) continue;

    int new = TrieMap_Add(edgeAliases, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    if(!new) {
      asprintf(reason, "Cannot use the same relationship variable '%s' for multiple patterns.", alias);
      res = AST_INVALID;
      break;
    }
  }

  TrieMap_Free(edgeAliases, TrieMap_NOP_CB);
  return res;
}

AST_Validation _Validate_RETURN_Clause(const AST_Query *ast, char **reason) {
  char *undefined_alias;

  if (!ast->returnNode) {
    return AST_VALID;
  }

  TrieMap *return_aliases = NewTrieMap();
  ReturnClause_ReferredEntities(ast->returnNode, return_aliases);
  
  AST_Validation res = _Validate_Aliases_In_Match_Clause(return_aliases, ast->matchNode, reason);
  TrieMap_Free(return_aliases, TrieMap_NOP_CB);
  if(res != AST_VALID) return res;

  // Retrieve all user-specified functions in RETURN clause.
  TrieMap *ref_functions = NewTrieMap();
  ReturnClause_ReferredFunctions(ast->returnNode, ref_functions);

  // Verify that referred functions exist.
  res = _AST_ValidateReferredFunctions(ref_functions, reason, true);

  TrieMap_Free(ref_functions, TrieMap_NOP_CB);

  return res;
}

AST_Validation _Validate_SET_Clause(const AST_Query *ast, char **reason) {
  char *undefined_alias;

  if (!ast->setNode) {
    return AST_VALID;
  }

  if (!ast->matchNode) {
    return AST_INVALID;
  }
  
  TrieMap *setAliases = NewTrieMap();
  SetClause_ReferredEntities(ast->setNode, setAliases);
  
  AST_Validation res = _Validate_Aliases_In_Match_Clause(setAliases, ast->matchNode, reason);
  TrieMap_Free(setAliases, TrieMap_NOP_CB);
  
  return res;
}

AST_Validation _Validate_WHERE_Clause(const AST_Query *ast, char **reason) {
  if (!ast->whereNode) return AST_VALID;
  if (!ast->matchNode) return AST_INVALID;

  TrieMap *where_aliases = NewTrieMap();
  WhereClause_ReferredEntities(ast->whereNode, where_aliases);

  AST_Validation res = _Validate_Aliases_In_Match_Clause(where_aliases, ast->matchNode, reason);
  TrieMap_Free(where_aliases, TrieMap_NOP_CB);

  if (res != AST_VALID) return res;

  // Retrieve all user-specified functions in WHERE clause.
  TrieMap *ref_functions = NewTrieMap();
  WhereClause_ReferredFunctions(ast->whereNode->filters, ref_functions);

  // Verify that referred functions exist.
  res = _AST_ValidateReferredFunctions(ref_functions, reason, false);

  TrieMap_Free(ref_functions, TrieMap_NOP_CB);

  return res;
}

AST_Validation AST_Validate(const AST_Query *ast, char **reason) {
  /* AST must include either a MATCH or CREATE clause. */
  if (!(ast->matchNode || ast->createNode || ast->mergeNode || ast->returnNode || ast->indexNode)) {
    asprintf(reason, "Query must specify either MATCH or CREATE clause");
    return AST_INVALID;
  }

  /* MATCH clause must be followed by either a CREATE or a RETURN clause. */
  if (ast->matchNode && !(ast->createNode || ast->returnNode || ast->setNode || ast->deleteNode || ast->mergeNode)) {
    asprintf(reason, "Query cannot conclude with MATCH (must be RETURN or an update clause)");
    return AST_INVALID;
  }

  if (_Validate_MATCH_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  if (_Validate_WHERE_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  if (_Validate_CREATE_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  if (_Validate_SET_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  if (_Validate_DELETE_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  if (_Validate_RETURN_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  return AST_VALID;
}

void AST_NameAnonymousNodes(AST_Query *ast) {
  int entity_id = 0;
  
  if(ast->matchNode)
    MatchClause_NameAnonymousNodes(ast->matchNode, &entity_id);

  if(ast->createNode)
    CreateClause_NameAnonymousNodes(ast->createNode, &entity_id);
}

bool AST_ReadOnly(const AST_Query *ast) {
  return !(ast->createNode != NULL ||
           ast->mergeNode != NULL ||
           ast->deleteNode != NULL ||
           ast->setNode != NULL ||
           ast->indexNode != NULL);
}

void Free_AST_Query(AST_Query *queryExpressionNode) {
  Free_AST_MatchNode(queryExpressionNode->matchNode);
  Free_AST_CreateNode(queryExpressionNode->createNode);
  Free_AST_MergeNode(queryExpressionNode->mergeNode);
  Free_AST_DeleteNode(queryExpressionNode->deleteNode);
  Free_AST_SetNode(queryExpressionNode->setNode);
  Free_AST_WhereNode(queryExpressionNode->whereNode);
  Free_AST_ReturnNode(queryExpressionNode->returnNode);
  Free_AST_SkipNode(queryExpressionNode->skipNode);
  Free_AST_OrderNode(queryExpressionNode->orderNode);
  free(queryExpressionNode);
}

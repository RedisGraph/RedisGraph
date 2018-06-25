#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../graph/graph_entity.h"
#include "./ast_arithmetic_expression.h"

AST_Query *New_AST_Query(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
                         AST_CreateNode *createNode, AST_SetNode *setNode,
                         AST_DeleteNode *deleteNode, AST_ReturnNode *returnNode,
                         AST_OrderNode *orderNode, AST_LimitNode *limitNode, AST_IndexNode *indexNode) {

  AST_Query *queryExpressionNode = (AST_Query *)malloc(sizeof(AST_Query));

  queryExpressionNode->matchNode = matchNode;
  queryExpressionNode->whereNode = whereNode;
  queryExpressionNode->createNode = createNode;
  queryExpressionNode->setNode = setNode;
  queryExpressionNode->deleteNode = deleteNode;
  queryExpressionNode->returnNode = returnNode;
  queryExpressionNode->orderNode = orderNode;
  queryExpressionNode->limitNode = limitNode;
  queryExpressionNode->indexNode = indexNode;

  return queryExpressionNode;
}

void Free_AST_Query(AST_Query *queryExpressionNode) {
  Free_AST_MatchNode(queryExpressionNode->matchNode);
  Free_AST_CreateNode(queryExpressionNode->createNode);
  // Free_AST_MergeNode(queryExpressionNode->mergeNode);
  Free_AST_DeleteNode(queryExpressionNode->deleteNode);
  Free_AST_SetNode(queryExpressionNode->setNode);
  Free_AST_WhereNode(queryExpressionNode->whereNode);
  Free_AST_ReturnNode(queryExpressionNode->returnNode);
  Free_AST_OrderNode(queryExpressionNode->orderNode);
  Free_AST_IndexNode(queryExpressionNode->indexNode);
  free(queryExpressionNode);
}

AST_Validation Validate_Aliases_In_Match_Clause(TrieMap *aliasesToCheck,
                                                const AST_MatchNode *match_node,
                                                char **undefined_alias) {
  /* Check that all the aliases that are in aliasesToCheck exists in the match clause */
  AST_Validation res = AST_VALID;
  TrieMap *matchAliases = NewTrieMap();
  MatchClause_ReferredNodes(match_node, matchAliases);

  TrieMapIterator *it = TrieMap_Iterate(aliasesToCheck, "", 0);
  char *ptr;
  tm_len_t len;
  void *value;

  while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
    if(TrieMap_Find(matchAliases, ptr, len) == TRIEMAP_NOTFOUND) {
      *undefined_alias = malloc(sizeof(char) * (strlen(" not defined") + len + 1));
      sprintf(*undefined_alias, "%s not defined", ptr);
      res = AST_INVALID;
      break;
    }
  }

  TrieMap_Free(matchAliases, TrieMap_NOP_CB);
  return res;
}

AST_Validation _Validate_CREATE_Clause(const AST_Query *ast, char **reason) {
  return AST_VALID;
}

AST_Validation _Validate_DELETE_Clause(const AST_Query *ast, char **reason) {
  AST_Validation res = AST_VALID;
  char *undefined_alias;

  if (!ast->deleteNode) {
    return AST_VALID;
  }

  if (!ast->matchNode) {
    return AST_INVALID;
  }

  TrieMap *delete_aliases = NewTrieMap();
  DeleteClause_ReferredNodes(ast->deleteNode, delete_aliases);

  if (Validate_Aliases_In_Match_Clause(delete_aliases,
                                       ast->matchNode,
                                       &undefined_alias) != AST_VALID) {
    *reason = undefined_alias;
    res = AST_INVALID;
  }
  
  TrieMap_Free(delete_aliases, TrieMap_NOP_CB);
  return res;
}

AST_Validation _Validate_MATCH_Clause(const AST_Query *ast, char **reason) {
  return AST_VALID;
}

AST_Validation _Validate_RETURN_Clause(const AST_Query *ast, char **reason) {
  char *undefined_alias;

  if (!ast->returnNode) {
    return AST_VALID;
  }

  if (!ast->matchNode) {
    return AST_INVALID;
  }

  TrieMap *return_aliases = NewTrieMap();
  ReturnClause_ReferredNodes(ast->returnNode, return_aliases);
  
  AST_Validation res = Validate_Aliases_In_Match_Clause(
      return_aliases, ast->matchNode, &undefined_alias);

  TrieMap_Free(return_aliases, TrieMap_NOP_CB);

  if (res != AST_VALID) {
    *reason = undefined_alias;
    return AST_INVALID;
  }

  return AST_VALID;
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
  SetClause_ReferredNodes(ast->setNode, setAliases);
  
  AST_Validation res = Validate_Aliases_In_Match_Clause(
      setAliases, ast->matchNode, &undefined_alias);
  
  TrieMap_Free(setAliases, TrieMap_NOP_CB);

  if (res != AST_VALID) {
    *reason = undefined_alias;
    return AST_INVALID;
  }

  return AST_VALID;
}

AST_Validation _Validate_WHERE_Clause(const AST_Query *ast, char **reason) {
  return AST_VALID;
}

AST_Validation AST_Validate(const AST_Query *ast, char **reason) {
  /* AST must include either a MATCH or CREATE clause. */
  if (!(ast->matchNode || ast->createNode || ast->indexNode)) {
    *reason = "Query must specify either MATCH or CREATE clause.";
    return AST_INVALID;
  }

  /* MATCH clause must be followed by either a CREATE or a RETURN clause. */
  if (ast->matchNode && !(ast->createNode || ast->returnNode || ast->setNode)) {
    *reason = "MATCH must be followed by either a RETURN or a CREATE clause.";
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

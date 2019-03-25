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
#include "../procedures/procedure.h"
#include "../arithmetic/repository.h"
#include "./ast_arithmetic_expression.h"
#include "../graph/entities/graph_entity.h"
#include "../arithmetic/arithmetic_expression.h"

/* Compares a triemap of user-specified functions with the registered functions we provide. */
static AST_Validation _AST_ValidateReferredFunctions(TrieMap *referred_functions,
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

/* Check that all the aliases that are in aliasesToCheck exists in the match clause */
static AST_Validation _Aliases_Defined(const AST *ast, char **undefined_alias) {
  AST_Validation res = AST_VALID;
  
  TrieMap *aliasesToCheck = NewTrieMap();
  ReturnClause_ReferredEntities(ast->returnNode, aliasesToCheck);
  OrderClause_ReferredEntities(ast->orderNode, aliasesToCheck);
  DeleteClause_ReferredEntities(ast->deleteNode, aliasesToCheck);
  SetClause_ReferredEntities(ast->setNode, aliasesToCheck);
  WhereClause_ReferredEntities(ast->whereNode, aliasesToCheck);
  UnwindClause_ReferredEntities(ast->unwindNode, aliasesToCheck);
  WithClause_ReferredEntities(ast->withNode, aliasesToCheck);

  TrieMap *definedAliases = ast->_aliasIDMapping;
  // TrieMap *definedAliases = ast-> NewTrieMap();
  // MatchClause_DefinedEntities(ast->matchNode, definedAliases);
  // UnwindClause_DefinedEntities(ast->unwindNode, definedAliases);
  // MergeClause_DefinedEntities(ast->mergeNode, definedAliases);
  // CreateClause_DefinedEntities(ast->createNode, definedAliases);

  TrieMapIterator *it = TrieMap_Iterate(aliasesToCheck, "", 0);
  char *alias;
  tm_len_t len;
  void *value;

  while(TrieMapIterator_Next(it, &alias, &len, &value)) {
    if(TrieMap_Find(definedAliases, alias, len) == TRIEMAP_NOTFOUND) {
      asprintf(undefined_alias, "%s not defined", alias);
      res = AST_INVALID;
      break;
    }
  }

  TrieMapIterator_Free(it);
  TrieMap_Free(aliasesToCheck, TrieMap_NOP_CB);
  return res;
}

static AST_Validation _Validate_CREATE_Clause(const AST *ast, char **reason) {
  if(!ast->createNode) return AST_VALID;
  AST_CreateNode *createNode = ast->createNode;
  
  int entityCount = Vector_Size(createNode->graphEntities);
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

static AST_Validation _Validate_DELETE_Clause(const AST *ast, char **reason) {

  if (!ast->deleteNode) {
    return AST_VALID;
  }

  if (!ast->matchNode) {
    return AST_INVALID;
  }
  
  return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause(const AST *ast, char **reason) {
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

  /* Verify that no alias appears in multiple independent patterns.
   * TODO We should introduce support for this when possible. */
  int patternCount = Vector_Size(ast->matchNode->patterns);
  if (Vector_Size(ast->matchNode->patterns) > 1) {
    TrieMap *reused_entities = NewTrieMap();
    Vector *pattern;
    AST_GraphEntity *elem;

    int pattern_ids[patternCount];
    for (int i = 0; i < patternCount; i ++) {
      pattern_ids[i] = i;
      Vector_Get(ast->matchNode->patterns, i, &pattern);
      for (int j = 0; j < Vector_Size(pattern); j ++) {
        Vector_Get(pattern, j, &elem);
        char *alias = elem->alias;
        if (!alias) continue;
        void *val = TrieMap_Find(reused_entities, alias, strlen(alias));
        // If this alias has been used in a previous pattern, emit an error.
        if (val != TRIEMAP_NOTFOUND && *(int*)val != i) {
          asprintf(reason, "Alias '%s' reused. Entities with the same alias may not be referenced in multiple patterns.", alias);
          res = AST_INVALID;
          break;
        }
        // Use the pattern index as a value, as it is valid to reuse a node alias within a pattern.
        TrieMap_Add(reused_entities, alias, strlen(alias), &pattern_ids[i], TrieMap_DONT_CARE_REPLACE);
      }
    }
    TrieMap_Free(reused_entities, TrieMap_NOP_CB);
  }

  return res;
}

static AST_Validation _Validate_RETURN_Clause(const AST *ast, char **reason) {
  char *undefined_alias;

  if (!ast->returnNode) return AST_VALID;

  // Retrieve all user-specified functions in RETURN clause.
  TrieMap *ref_functions = NewTrieMap();
  ReturnClause_ReferredFunctions(ast->returnNode, ref_functions);

  // Verify that referred functions exist.
  AST_Validation res = _AST_ValidateReferredFunctions(ref_functions, reason, true);
  int function_count = ref_functions->cardinality;

  TrieMap_Free(ref_functions, TrieMap_NOP_CB);
  if (res != AST_VALID || function_count == 0) return res;

  // Verify that we are not attempting to invalidly aggregate graph entities.
  AST_ArithmeticExpressionOP *funcs[function_count];
  int aggregate_count = ReturnClause_AggregateFunctions(ast->returnNode, funcs);

  if (aggregate_count == 0) return AST_VALID;

  // We are only interested in MATCH entities; as variadics introduced through
  // clauses like UNWIND can be validly aggregated.
  TrieMap *entities = NewTrieMap();
  MatchClause_DefinedEntities(ast->matchNode, entities);

  for (int i = 0; i < aggregate_count; i ++) {
    AST_ArithmeticExpressionOP *func = funcs[i];
    // COUNT calls may validly aggregate nodes and edges
    if (!strcasecmp(func->function, "count")) continue;

    for(int j = 0; j < Vector_Size(func->args); j ++) {
      AST_ArithmeticExpressionNode *child;
      Vector_Get(func->args, j, &child);
      // Check if the child is a variadic with no property specified, which may
      // refer to a graph entity
      if (EXPRESSION_IS_ALIAS(child)) {
        char *alias = child->operand.variadic.alias;
        // Look up alias in the entities defined by the MATCH clause
        if(TrieMap_Find(entities, alias, strlen(alias)) != TRIEMAP_NOTFOUND) {
          asprintf(reason, "Nodes and edges cannot be aggregated by functions other than COUNT.");
          res = AST_INVALID;
          break;
        }
      }
    }
    if (res == AST_INVALID) break;
  }

  TrieMap_Free(entities, TrieMap_NOP_CB);

  return res;
}

static AST_Validation _Validate_SET_Clause(const AST *ast, char **reason) {
  char *undefined_alias;

  if (!ast->setNode) {
    return AST_VALID;
  }

  // if (!ast->matchNode && !ast->mergeNode) {
  //   return AST_INVALID;
  // }

  return AST_VALID;
}

static AST_Validation _Validate_WHERE_Clause(const AST *ast, char **reason) {
  if (!ast->whereNode) return AST_VALID;
  // MATCH, CALL, UNWIND can introduce data.
  if (!(ast->matchNode || ast->callNode || ast->unwindNode)) return AST_INVALID;

  // Retrieve all user-specified functions in WHERE clause.
  TrieMap *ref_functions = NewTrieMap();
  WhereClause_ReferredFunctions(ast->whereNode->filters, ref_functions);

  // Verify that referred functions exist.
  AST_Validation res = _AST_ValidateReferredFunctions(ref_functions, reason, false);

  TrieMap_Free(ref_functions, TrieMap_NOP_CB);

  return res;
}

static AST_Validation _Validate_CALL_Clause(const AST *ast, char **reason) {
  if (!ast->callNode) return AST_VALID;
  // Make sure refereed procedure exists.
  ProcedureCtx *proc = Proc_Get(ast->callNode->procedure);
  if(!proc) {
    asprintf(reason, "There is no procedure with the name `%s` registered for this database instance. Please ensure you've spelled the procedure name correctly.",
             ast->callNode->procedure);
    return AST_INVALID;
  }

  if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT && proc->argc != array_len(ast->callNode->arguments)) {
    asprintf(reason, "Procedure call does not provide the required number of arguments: got %d expected %d. ",
             array_len(ast->callNode->arguments),
             proc->argc);

    Proc_Free(proc);
    return AST_INVALID;
  }

  // Make sure yield doesn't refers to unknown output.
  if(ast->callNode->yield) {
    uint i = 0;
    uint j = 0;
    char *yield;
    uint call_yield_len = array_len(ast->callNode->yield);
    uint proc_output_len = array_len(proc->output);

    for(i = 0; i < call_yield_len; i++) {
      yield = ast->callNode->yield[i];
      for(j = 0; j < proc_output_len; j++) {
        if(strcmp(yield, proc->output[j]->name) == 0) break;
      }
      if(j == proc_output_len) {
        // Didn't managed to match current yield against procedure output.
        break;
      }
    }

    if(i < call_yield_len) {
      Proc_Free(proc);
      asprintf(reason, "Unknown procedure output: `%s`", yield);
      return AST_INVALID;
    }
  }

  Proc_Free(proc);
  return AST_VALID;
}

AST *AST_New(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
                         AST_CreateNode *createNode, AST_MergeNode *mergeNode,
                         AST_SetNode *setNode, AST_DeleteNode *deleteNode,
                         AST_ReturnNode *returnNode, AST_OrderNode *orderNode,
                         AST_SkipNode *skipNode, AST_LimitNode *limitNode,
                         AST_IndexNode *indexNode, AST_UnwindNode *unwindNode,
                         AST_ProcedureCallNode *callNode) {
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
  ast->callNode = callNode;
  ast->withNode = NULL;
  ast->_aliasIDMapping = NULL;
  return ast;
}

AST_Validation AST_Validate(const AST *ast, char **reason) {
  /* AST must include either a MATCH or CREATE clause. */
  // if (!(ast->matchNode || ast->createNode || ast->mergeNode || ast->returnNode || ast->indexNode || ast->withNode)) {
    // asprintf(reason, "Query must specify either MATCH or CREATE clause");
    // return AST_INVALID;
  // }

  /* MATCH clause must be followed by either a CREATE or a RETURN clause. */
  if (ast->matchNode && !(ast->createNode || ast->returnNode || ast->setNode || ast->deleteNode || ast->mergeNode || ast->withNode)) {
    asprintf(reason, "Query cannot conclude with MATCH (must be RETURN or an update clause)");
    return AST_INVALID;
  }

  if(_Aliases_Defined(ast, reason) != AST_VALID) {
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

  if(_Validate_CALL_Clause(ast, reason) != AST_VALID) {
    return AST_INVALID;
  }

  return AST_VALID;
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
  TrieMap *definedEntities = AST_Identifiers(ast);
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

TrieMap* AST_CollectEntityReferences(AST **ast_arr) {
  TrieMap *alias_references = NewTrieMap();

  uint ast_count = array_len(ast_arr);
  for (uint i = 0; i < ast_count; i ++) {
      const AST *ast = ast_arr[i];
      // Get unique aliases from clauses that can introduce nodes and edges.
      MatchClause_DefinedEntities(ast->matchNode, alias_references);
      CreateClause_DefinedEntities(ast->createNode, alias_references);
      ProcedureCallClause_DefinedEntities(ast->callNode, alias_references);

      // TODO May need to collect alias redefinitions from WITH clauses
  }
  return alias_references;
}

// Returns a triemap of all identifiers defined by ast.
TrieMap* AST_Identifiers(const AST *ast) {
  TrieMap *identifiers = NewTrieMap();
  MatchClause_DefinedEntities(ast->matchNode, identifiers);
  ReturnClause_DefinedEntities(ast->returnNode, identifiers);
  WithClause_DefinedEntities(ast->withNode, identifiers);
  CreateClause_DefinedEntities(ast->createNode, identifiers);
  UnwindClause_DefinedEntities(ast->unwindNode, identifiers);
  ProcedureCallClause_DefinedEntities(ast->callNode, identifiers);
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
    Free_AST_ProcedureCallNode(ast[i]->callNode);
    Free_AST_WithNode(ast[i]->withNode);
    if(ast[i]->_aliasIDMapping) TrieMap_Free(ast[i]->_aliasIDMapping, NULL);
    rm_free(ast[i]);
  }
  array_free(ast);
}

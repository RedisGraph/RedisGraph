/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "query_executor.h"
#include "graph/graph.h"
#include "graph/entities/node.h"
#include "stores/store.h"
#include "util/vector.h"
#include "parser/grammar.h"
#include "arithmetic/agg_ctx.h"
#include "arithmetic/repository.h"
#include "parser/parser_common.h"

static void _returnClause_ExpandCollapsedNodes(GraphContext *gc, AST_Query *ast) {
    assert(gc);

     /* Expanding the RETURN clause is a two phase operation:
      * 1. Scan through every arithmetic expression within the original
      * RETURN clause and add it to a temporary vector,
      * if we bump into an asterisks marker indicating we should expand
      * all nodes, relations and paths within the MATCH clause, add thoese
      * to the temporary vector.
      * 
      * 2. Scanning though the temporary vector we expand each collapsed entity
      * this will form the final RETURN clause. */

    AST_ReturnNode *returnClause = ast->returnNode;
    Vector *elementsToExpand = NewVector(AST_ReturnElementNode*, Vector_Size(returnClause->returnElements));

    // Scan through return elements, see if we can find '*' marker.
    size_t returnElementCount = Vector_Size(returnClause->returnElements);
    for(int i = 0; i < returnElementCount; i++) {
        AST_ReturnElementNode *retElement;
        Vector_Get(returnClause->returnElements, i, &retElement);

        if(retElement->asterisks) {
            /* Expand with "RETURN *" queries.
             * Extract all entities from MATCH clause and add them to RETURN clause.
             * These will get expended later on. */

            char *ptr;
            tm_len_t len;
            void *value;
            TrieMap *matchEntities = NewTrieMap();
            MatchClause_ReferredEntities(ast->matchNode, matchEntities);
            TrieMapIterator *it = TrieMap_Iterate(matchEntities, "", 0);
            while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
                AST_ArithmeticExpressionNode *arNode = New_AST_AR_EXP_VariableOperandNode(ptr, NULL);
                AST_ReturnElementNode *returnEntity = New_AST_ReturnElementNode(arNode, NULL);
                Vector_Push(elementsToExpand, returnEntity);
            }

            TrieMapIterator_Free(it);
            TrieMap_Free(matchEntities, TrieMap_NOP_CB);
            Free_AST_ReturnElementNode(retElement);
        }
        else Vector_Push(elementsToExpand, retElement);
    }

    Vector *expandReturnElements = NewVector(AST_ReturnElementNode*, Vector_Size(elementsToExpand));

    /* Scan return clause, search for collapsed nodes. */
    for(int i = 0; i < Vector_Size(elementsToExpand); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(elementsToExpand, i, &ret_elem);
        AST_ArithmeticExpressionNode *exp = ret_elem->exp;
        
        /* Detect collapsed entity,
         * A collapsed entity is represented by an arithmetic expression
         * of AST_AR_EXP_OPERAND type, 
         * The operand type should be AST_AR_EXP_VARIADIC,
         * lastly property should be missing. */

        if(exp->type == AST_AR_EXP_OPERAND &&
            exp->operand.type == AST_AR_EXP_VARIADIC &&
            exp->operand.variadic.property == NULL) {
            
            /* Return clause doesn't contains entity's label,
             * Find collapsed entity's label. */
            AST_GraphEntity *collapsed_entity = MatchClause_GetEntity(ast->matchNode, exp->operand.variadic.alias);

            /* We have already validated the query at this point, so all entity lookups should succeed. */
            assert(collapsed_entity);

            /* Find label's properties. */
            LabelStoreType store_type = (collapsed_entity->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
            LabelStore *store;
                        
            if(collapsed_entity->label) {
                /* Collapsed entity has a label. */
                store = GraphContext_GetStore(gc, collapsed_entity->label, store_type);
            } else {
                /* Entity does have a label, Consult with "ALL" store. */
                store = GraphContext_AllStore(gc, store_type);
            }

            void *ptr = NULL;       /* Label store property value, (not in use). */
            char *prop = NULL;      /* Entity property. */
            tm_len_t prop_len = 0;  /* Length of entity's property. */
            AST_ArithmeticExpressionNode *expanded_exp;
            AST_ReturnElementNode *retElem;
            if(!store || store->properties->cardinality == 0) {
                /* Label store was not found or
                 * label doesn't have any properties.
                 * Create a fake return element. */
                expanded_exp = New_AST_AR_EXP_ConstOperandNode(SI_StringVal(""));
                // Incase an alias is given use it, otherwise use the variable name.
                if(ret_elem->alias) retElem = New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);
                else retElem = New_AST_ReturnElementNode(expanded_exp, exp->operand.variadic.alias);
                Vector_Push(expandReturnElements, retElem);
            } else {
                TrieMapIterator *it = TrieMap_Iterate(store->properties, "", 0);
                while(TrieMapIterator_Next(it, &prop, &prop_len, &ptr)) {
                    prop[prop_len] = 0;
                    /* Create a new return element foreach property. */
                    expanded_exp = New_AST_AR_EXP_VariableOperandNode(collapsed_entity->alias, prop);
                    retElem = New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);
                    Vector_Push(expandReturnElements, retElem);
                }
                TrieMapIterator_Free(it);
            }

            /* Discard collapsed return element. */
            Free_AST_ReturnElementNode(ret_elem);
        } else {
            Vector_Push(expandReturnElements, ret_elem);
        }
    }
    /* Override previous return clause. */
    Vector_Free(elementsToExpand);
    Vector_Free(returnClause->returnElements);
    returnClause->returnElements = expandReturnElements;
}

static void _inlineProperties(AST_Query *ast) {
    /* Migrate inline filters to WHERE clause. */
    if(!ast->matchNode) return;
    // Vector *entities = ast->matchNode->graphEntities;
    Vector *entities = ast->matchNode->_mergedPatterns;

    char *alias;
    char *property;
    AST_ArithmeticExpressionNode *lhs;
    AST_ArithmeticExpressionNode *rhs;

    /* Foreach entity. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        alias = entity->alias;

        Vector *properties = entity->properties;
        if(properties == NULL) {
            continue;
        }

        /* Foreach property. */
        for(int j = 0; j < Vector_Size(properties); j+=2) {
            SIValue *key;
            SIValue *val;

            // Build the left-hand filter value from the node alias and property
            Vector_Get(properties, j, &key);
            property = key->stringval;
            lhs = New_AST_AR_EXP_VariableOperandNode(alias, property);

            // Build the right-hand filter value from the specified constant
            // TODO can update grammar so that this constant is already an ExpressionNode
            // instead of an SIValue
            Vector_Get(properties, j+1, &val);
            rhs = New_AST_AR_EXP_ConstOperandNode(*val);

            AST_FilterNode *filterNode = New_AST_PredicateNode(lhs, EQ, rhs);
            
            /* Create WHERE clause if missing. */
            if(ast->whereNode == NULL) {
                ast->whereNode = New_AST_WhereNode(filterNode);
            } else {
                /* Introduce filter with AND operation. */
                AST_FilterNode *left = ast->whereNode->filters;
                AST_FilterNode *right = filterNode;
                ast->whereNode->filters = New_AST_ConditionNode(left, AND, right);
            }
        }
    }
}

/* Shares merge pattern with match clause. */
static void _replicateMergeClauseToMatchClause(AST_Query *ast) {    
    assert(ast->mergeNode && !ast->matchNode);

    /* Match node is expecting a vector of vectors,
     * and so we have to wrap merge graph entities vector
     * within another vector
     * wrappedEntities will be freed by match clause. */
    Vector *wrappedEntities = NewVector(Vector*, 1);
    Vector_Push(wrappedEntities, ast->mergeNode->graphEntities);
    ast->matchNode = New_AST_MatchNode(wrappedEntities);
}

//------------------------------------------------------------------------------

/* Construct an expression tree foreach none aggregated term.
 * Returns a vector of none aggregated expression trees. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count) {
    *expressions = malloc(sizeof(AR_ExpNode *) * Vector_Size(return_node->returnElements));
    *expressions_count = 0;

    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(return_node->returnElements, i, &returnElement);

        AR_ExpNode *expression = AR_EXP_BuildFromAST(returnElement->exp);
        if(!AR_EXP_ContainsAggregation(expression, NULL)) {
            (*expressions)[*expressions_count] = expression;
            (*expressions_count)++;
        }
    }
}

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    return Query_Parse(query, qLen, errMsg);
}

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, AST_Query *ast) {
    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        free(reason);
        return AST_INVALID;
    }
    return AST_VALID;
}

void ModifyAST(GraphContext *gc, AST_Query *ast) {
    if(ast->mergeNode) {
        /* Create match clause which will try to match 
         * against pattern specified within merge clause. */
        _replicateMergeClauseToMatchClause(ast);
    }

    if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
        /* Expand collapsed nodes. */
        _returnClause_ExpandCollapsedNodes(gc, ast);
    }

    AST_NameAnonymousNodes(ast);

    _inlineProperties(ast);
}

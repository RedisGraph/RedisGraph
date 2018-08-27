/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "assert.h"
#include "graph/graph.h"
#include "graph/node.h"
#include "stores/store.h"
#include "rmutil/vector.h"
#include "rmutil/util.h"
#include "query_executor.h"
#include "parser/grammar.h"
#include "arithmetic/agg_ctx.h"
#include "arithmetic/repository.h"
#include "parser/parser_common.h"

QueryContext* QueryContext_New(RedisModuleCtx *ctx, RedisModuleBlockedClient *bc, AST_Query* ast, RedisModuleString *graphName) {
    QueryContext* context = malloc(sizeof(QueryContext));
    context->bc = bc;
    context->ast = ast;
    context->graphName = graphName;
    RedisModule_RetainString(ctx, context->graphName);
    return context;
}

void QueryContext_Free(QueryContext* ctx) {
    RedisModule_Free(ctx->graphName);
    free(ctx);
}

/* Construct an expression tree foreach none aggregated term.
 * Returns a vector of none aggregated expression trees. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, QueryGraph *g) {
    *expressions = malloc(sizeof(AR_ExpNode *) * Vector_Size(return_node->returnElements));
    *expressions_count = 0;

    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(return_node->returnElements, i, &returnElement);

        AR_ExpNode *expression = AR_EXP_BuildFromAST(returnElement->exp, g);
        if(!AR_EXP_ContainsAggregation(expression, NULL)) {
            (*expressions)[*expressions_count] = expression;
            (*expressions_count)++;
        }
    }
}

AST_Validation ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_Query *ast, const char *graphName, char **reason) {
     /* Create a new return clause */
    Vector *expandReturnElements = NewVector(AST_ReturnElementNode*, Vector_Size(ast->returnNode->returnElements));

    /* Scan return clause, search for collapsed nodes. */
    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(ast->returnNode->returnElements, i, &ret_elem);
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

            /* Failed to find collapsed entity. */
            if(collapsed_entity == NULL) {
                /* Invalid query, return clause refers to non-existent entity. */
                asprintf(reason, "%s not defined", exp->operand.variadic.alias);

                /* We've partially freed return elements and partially constructed
                 * some replacement elements, so some cleanup is required. */

                /* Free the replacement elements that have been built so far. */
                for (int j = 0; i < Vector_Size(expandReturnElements); j ++) {
                    Vector_Get(expandReturnElements, i, &ret_elem);
                    Free_AST_ReturnElementNode(ret_elem);
                }
                Vector_Free(expandReturnElements);

                /* Free the remaining elements in the return clause. */
                for(; i < Vector_Size(ast->returnNode->returnElements); i++) {
                    AST_ReturnElementNode *ret_elem;
                    Vector_Get(ast->returnNode->returnElements, i, &ret_elem);
                    Free_AST_ReturnElementNode(ret_elem);
                }
                Vector_Free(ast->returnNode->returnElements);
                free(ast->returnNode);
                ast->returnNode = NULL;

                return AST_INVALID;
            }

            /* Find label's properties. */
            LabelStoreType store_type = (collapsed_entity->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
            LabelStore *store;
                        
            if(collapsed_entity->label) {
                /* Collapsed entity has a label. */
                store = LabelStore_Get(ctx, store_type, graphName, collapsed_entity->label);
            } else {
                /* Entity does have a label, Consult with "ALL" store. */
                store = LabelStore_Get(ctx, store_type, graphName, NULL);
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
    Vector_Free(ast->returnNode->returnElements);
    ast->returnNode->returnElements = expandReturnElements;

    return AST_VALID;
}

/* Shares merge pattern with match clause. */
void _replicateMergeClauseToMatchClause(AST_Query *ast) {    
    assert(ast->mergeNode && !ast->matchNode);

    /* Match node is expecting a vector of vectors,
     * and so we have to wrap merge graph entities vector
     * within another vector
     * wrappedEntities will be freed by match clause. */
    Vector *wrappedEntities = NewVector(Vector*, 1);
    Vector_Push(wrappedEntities, ast->mergeNode->graphEntities);
    ast->matchNode = New_AST_MatchNode(wrappedEntities);
}

void inlineProperties(AST_Query *ast) {
    /* Migrate inline filters to WHERE clause. */
    if(!ast->matchNode) return;
    // Vector *entities = ast->matchNode->graphEntities;
    Vector *entities = ast->matchNode->_mergedPatterns;

    /* Foreach entity. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        Vector *properties = entity->properties;
        if(properties == NULL) {
            continue;
        }

        /* Foreach property. */
        for(int j = 0; j < Vector_Size(properties); j+=2) {
            SIValue *key;
            SIValue *val;

            Vector_Get(properties, j, &key);
            Vector_Get(properties, j+1, &val);

            const char *alias = entity->alias;
            const char *property = key->stringval;

            AST_FilterNode *filterNode = New_AST_ConstantPredicateNode(alias, property, EQ, *val);
            
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

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    return Query_Parse(query, qLen, errMsg);
}

AST_Validation ModifyAST(RedisModuleCtx *ctx, AST_Query *ast, const char *graph_name, char **reason) {
    if(ast->mergeNode) {
        /* Create match clause which will try to match 
         * against pattern specified within merge clause. */
        _replicateMergeClauseToMatchClause(ast);
    }

    AST_NameAnonymousNodes(ast);

    if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
        /* Expand collapsed nodes. */
        AST_Validation res = ReturnClause_ExpandCollapsedNodes(ctx, ast, graph_name, reason) != AST_VALID;
        if (res != AST_VALID) return AST_INVALID;
    }

    inlineProperties(ast);

    return AST_VALID;
}

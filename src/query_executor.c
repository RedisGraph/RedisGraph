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
#include "schema/schema.h"
#include "util/arr.h"
#include "util/vector.h"
#include "parser/grammar.h"
#include "arithmetic/agg_ctx.h"
#include "arithmetic/repository.h"
#include "parser/parser_common.h"
#include "../deps/libcypher-parser/lib/src/cypher-parser.h"

static void _inlineProperties(AST *ast) {
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
static void _replicateMergeClauseToMatchClause(AST *ast) {
    assert(ast->mergeNode && !ast->matchNode);

    /* Match node is expecting a vector of vectors,
     * and so we have to wrap merge graph entities vector
     * within another vector
     * wrappedEntities will be freed by match clause. */
    Vector *wrappedEntities = NewVector(Vector*, 1);
    Vector_Push(wrappedEntities, ast->mergeNode->graphEntities);
    ast->matchNode = New_AST_MatchNode(wrappedEntities);
}

ReturnElementNode* _NewReturnElementNode(const char *alias, AR_ExpNode *exp) {
    ReturnElementNode *elem = rm_malloc(sizeof(ReturnElementNode));
    elem->alias = alias;
    elem->exp = exp;
    return elem;
}

void _oldExpandCollapsedNodes(void) {
    AST *ast = AST_GetFromLTS();
    char buffer[256];
    GraphContext *gc = GraphContext_GetFromLTS();

    /* Expanding the RETURN clause is a two phase operation:
     * 1. Scan through every arithmetic expression within the original
     * RETURN clause and add it to a temporary vector,
     * if we bump into an asterisks marker indicating we should expand
     * all nodes, relations and paths, add thoese to the temporary vector.
     *
     * 2. Scanning though the temporary vector we expand each collapsed entity
     * this will form the final RETURN clause. */

    AST_ReturnNode *returnClause = ast->returnNode;
    size_t returnElementCount = array_len(returnClause->returnElements);
    AST_ReturnElementNode **elementsToExpand = array_new(AST_ReturnElementNode*, returnElementCount);

    TrieMap *identifiers = NewTrieMap();
    MatchClause_DefinedEntities(ast->matchNode, identifiers);
    CreateClause_DefinedEntities(ast->createNode, identifiers);

    // Scan through return elements, see if we can find '*' marker.
    for(int i = 0; i < returnElementCount; i++) {
        AST_ReturnElementNode *retElement = returnClause->returnElements[i];
        if(retElement->asterisks) {
            // TODO: '*' can not be mixed with any other expression!
            array_clear(elementsToExpand);
            /* Expand with "RETURN *" queries.
             * Extract all entities from MATCH clause and add them to RETURN clause.
             * These will get expended later on. */
            char *ptr;
            tm_len_t len;
            void *value;
            TrieMapIterator *it = TrieMap_Iterate(identifiers, "", 0);
            while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
                AST_GraphEntity *entity = (AST_GraphEntity*)value;
                if(entity->anonymous) continue;

                len = MIN(255, len);
                memcpy(buffer, ptr, len);
                buffer[len] = '\0';

                AST_ArithmeticExpressionNode *arNode = New_AST_AR_EXP_VariableOperandNode(buffer, NULL);
                AST_ReturnElementNode *returnEntity = New_AST_ReturnElementNode(arNode, NULL);
                elementsToExpand = array_append(elementsToExpand, returnEntity);
            }

            TrieMapIterator_Free(it);
            Free_AST_ReturnElementNode(retElement);
            break;
        }
        else {
            elementsToExpand = array_append(elementsToExpand, retElement);
        }
    }

    AST_ReturnElementNode **expandReturnElements = array_new(AST_ReturnElementNode*, array_len(elementsToExpand));

    /* Scan return clause, search for collapsed nodes. */
    for(int i = 0; i < array_len(elementsToExpand); i++) {
        AST_ReturnElementNode *ret_elem = elementsToExpand[i];
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
            char *alias = exp->operand.variadic.alias;
            AST_GraphEntity *collapsed_entity = TrieMap_Find(identifiers, alias, strlen(alias));
            if(collapsed_entity == TRIEMAP_NOTFOUND) {
                // It is possible that the current alias
                // represent an expression such as in the case of an unwind clause.
                expandReturnElements = array_append(expandReturnElements, ret_elem);
                continue;
            }

            /* Find label's properties. */
            SchemaType schema_type = (collapsed_entity->t == N_ENTITY) ? SCHEMA_NODE : SCHEMA_EDGE;
            Schema *schema;

            if(collapsed_entity->label) {
                /* Collapsed entity has a label. */
                schema = GraphContext_GetSchema(gc, collapsed_entity->label, schema_type);
            } else {
                /* Entity does have a label, Consult with unified schema. */
                schema = GraphContext_GetUnifiedSchema(gc, schema_type);
            }

            void *ptr = NULL;       /* schema property value, (not in use). */
            char *prop = NULL;      /* Entity property. */
            tm_len_t prop_len = 0;  /* Length of entity's property. */
            AST_ArithmeticExpressionNode *expanded_exp;
            AST_ReturnElementNode *retElem;
            if(!schema || Schema_AttributeCount(schema) == 0) {
                /* Schema missing or
                 * label doesn't have any properties.
                 * Create a fake return element. */
                expanded_exp = New_AST_AR_EXP_ConstOperandNode(SI_ConstStringVal(""));
                // Incase an alias is given use it, otherwise use the variable name.
                if(ret_elem->alias) retElem = New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);
                else retElem = New_AST_ReturnElementNode(expanded_exp, exp->operand.variadic.alias);
                expandReturnElements = array_append(expandReturnElements, retElem);
            } else {
                TrieMapIterator *it = TrieMap_Iterate(schema->attributes, "", 0);
                while(TrieMapIterator_Next(it, &prop, &prop_len, &ptr)) {
                    prop_len = MIN(255, prop_len);
                    memcpy(buffer, prop, prop_len);
                    buffer[prop_len] = '\0';

                    /* Create a new return element foreach property. */
                    expanded_exp = New_AST_AR_EXP_VariableOperandNode(collapsed_entity->alias, buffer);
                    retElem = New_AST_ReturnElementNode(expanded_exp, ret_elem->alias);
                    expandReturnElements = array_append(expandReturnElements, retElem);
                }
                TrieMapIterator_Free(it);
            }
            /* Discard collapsed return element. */
            Free_AST_ReturnElementNode(ret_elem);
        } else {
            expandReturnElements = array_append(expandReturnElements, ret_elem);
        }
    }

    /* Override previous return clause. */
    TrieMap_Free(identifiers, TrieMap_NOP_CB);
    array_free(elementsToExpand);
    array_free(returnClause->returnElements);
    returnClause->returnElements = expandReturnElements;
}

AST* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    return Query_Parse(query, qLen, errMsg);
}

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const cypher_astnode_t *ast) {
    char *reason;
    AST_Validation res = NEWAST_Validate(ast, &reason);
    if (res != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        free(reason);
        return AST_INVALID;
    }
    return AST_VALID;
}

/* Counts the number of right to left edges,
 * if it's greater than half the number of edges in pattern
 * return true.*/
static bool _AST_should_reverse_pattern(Vector *pattern) {
    int transposed = 0; // Number of transposed edges
    int edge_count = 0; // Total number of edges.
    int pattern_length = Vector_Size(pattern);

    // Count how many edges are going right to left.
    for(int i = 0; i < pattern_length; i++) {
        AST_GraphEntity *match_element;
        Vector_Get(pattern, i, &match_element);

        if(match_element->t != N_LINK) continue;

        edge_count++;
        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        if(edge->direction ==  N_RIGHT_TO_LEFT) transposed++;
    }

    // No edges.
    if(edge_count == 0) return false;
    return (transposed > edge_count/2);
}

/* Construct a new MATCH clause by cloning the current one
 * and reversing traversal patterns to reduce matrix transpose
 * operation. */
static void _AST_reverse_match_patterns(AST *ast) {
    size_t pattern_count = Vector_Size(ast->matchNode->patterns);
    Vector *patterns = NewVector(Vector*, pattern_count);

    for(int i = 0; i < pattern_count; i++) {
        Vector *pattern;
        Vector_Get(ast->matchNode->patterns, i, &pattern);

        size_t pattern_length = Vector_Size(pattern);
        Vector *v = NewVector(AST_GraphEntity*, pattern_length);

        if(!_AST_should_reverse_pattern(pattern)) {
            // No need to reverse, simply clone pattern.
            for(int j = 0; j < pattern_length; j++) {
                AST_GraphEntity *e;
                Vector_Get(pattern, j, &e);
                e = Clone_AST_GraphEntity(e);
                Vector_Push(v, e);
            }
        }
        else {
            /* Reverse pattern:
             * Create a new pattern where edges been reversed.
             * Nodes should be introduced in reverse order:
             * (C)<-[B]-(A)
             * (A)-[B]->(C) */
            for(int j = pattern_length-1; j >= 0; j--) {
                AST_GraphEntity *e;
                Vector_Get(pattern, j, &e);
                e = Clone_AST_GraphEntity(e);

                if(e->t == N_LINK) {
                    AST_LinkEntity *l = (AST_LinkEntity*)e;
                    // Reverse pattern.
                    if(l->direction == N_RIGHT_TO_LEFT) l->direction = N_LEFT_TO_RIGHT;
                    else l->direction = N_RIGHT_TO_LEFT;
                }
                Vector_Push(v, e);
            }
        }
        Vector_Push(patterns, v);
    }

    // Free old MATCH clause.
    Free_AST_MatchNode(ast->matchNode);
    // Update AST MATCH clause.
    ast->matchNode = New_AST_MatchNode(patterns);
}

static void _AST_optimize_traversal_direction(AST *ast) {
    /* Inspect each MATCH pattern,
     * see if the number of edges going from right to left ()<-[]-()
     * is greater than the number of edges going from left to right ()-[]->()
     * in which case it's worth reversing the pattern to reduce
     * matrix transpose operations. */

    bool should_reverse = false;
    size_t pattern_count = Vector_Size(ast->matchNode->patterns);
    for(int i = 0; i < pattern_count; i++) {
        Vector *pattern;
        Vector_Get(ast->matchNode->patterns, i, &pattern);

        if(_AST_should_reverse_pattern(pattern)) {
            should_reverse = true;
            break;            
        }
    }

    if(should_reverse) _AST_reverse_match_patterns(ast);
}

void ModifyAST(GraphContext *gc, AST *ast, const cypher_parse_result_t *new_ast) {
    if(ast->matchNode) _AST_optimize_traversal_direction(ast);

    if(ast->mergeNode) {
        /* Create match clause which will try to match 
         * against pattern specified within merge clause. */
        _replicateMergeClauseToMatchClause(ast);
    }

    // if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
    if(NEWAST_ReturnClause_ContainsCollapsedNodes(new_ast) == 1) {
        /* Expand collapsed nodes. */
        ExpandCollapsedNodes(ast);
    }

    AST_NameAnonymousNodes(ast);
    _inlineProperties(ast);
}

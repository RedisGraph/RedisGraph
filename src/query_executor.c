/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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
    ReturnElementNode *ret = malloc(sizeof(ReturnElementNode));
    ret->alias = alias;
    ret->exp = exp;

    return ret;
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
                // Use the return entity's alias if provided, otherwise use the variable's
                // name (which is never null).
                char *ret_alias = (ret_elem->alias) ? strdup(ret_elem->alias) : strdup(alias);
                retElem = New_AST_ReturnElementNode(expanded_exp, ret_alias);
                expandReturnElements = array_append(expandReturnElements, retElem);
            } else {
                TrieMapIterator *it = TrieMap_Iterate(schema->attributes, "", 0);
                while(TrieMapIterator_Next(it, &prop, &prop_len, &ptr)) {
                    prop_len = MIN(255, prop_len);
                    memcpy(buffer, prop, prop_len);
                    buffer[prop_len] = '\0';

                    /* Create a new return element foreach property. */
                    expanded_exp = New_AST_AR_EXP_VariableOperandNode(collapsed_entity->alias, buffer);
                    char *ret_alias = (ret_elem->alias) ? strdup(ret_elem->alias) : NULL;
                    retElem = New_AST_ReturnElementNode(expanded_exp, ret_alias);
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

void ExpandCollapsedNodes(NEWAST *ast) {

    _oldExpandCollapsedNodes();
    char buffer[256];
    GraphContext *gc = GraphContext_GetFromLTS();

    // TODO asterisks handling
    unsigned int return_expression_count = array_len(ast->return_expressions);
    ReturnElementNode **expandReturnElements = array_new(ReturnElementNode*, return_expression_count);

    /* Scan return clause, search for collapsed nodes. */
    for (unsigned int i = 0; i < return_expression_count; i++) {
        ReturnElementNode *elem = ast->return_expressions[i];
        // assert (elem->t == A_EXPRESSION);

        AR_ExpNode *exp = elem->exp;

        /* Detect collapsed entity,
         * A collapsed entity is represented by an arithmetic expression
         * of AR_EXP_OPERAND type,
         * The operand type should be AST_AR_EXP_VARIADIC,
         * lastly property should be missing. */

        if(exp->type == AR_EXP_OPERAND &&
            exp->operand.type == AR_EXP_VARIADIC &&
            exp->operand.variadic.entity_prop == NULL) {

            /* Return clause doesn't contains entity's label,
             * Find collapsed entity's label. */
            char *alias = (char*)elem->alias;
            // char *alias = exp->operand.variadic.entity_alias;
            unsigned int id = NEWAST_GetAliasID(ast, alias);
            NEWAST_GraphEntity *collapsed_entity = NEWAST_GetEntity(ast, id);
            // Entity was an expression rather than a node or edge
            // if (collapsed_entity->t != A_ENTITY) continue;

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

            AR_ExpNode *expanded_exp;
            ReturnElementNode *retElem;
            if(Schema_AttributeCount(schema) == 0) {
                /* Schema missing or
                 * label doesn't have any properties.
                 * Create a fake return element. */
                expanded_exp = AR_EXP_NewConstOperandNode(SI_ConstStringVal(""));
                // TODO capture alias
                // Incase an alias is given use it, otherwise use the variable name.
                // retElem = New_AST_Entity(elem->alias, A_EXPRESSION, expanded_exp);
                retElem = _NewReturnElementNode(elem->alias, expanded_exp);
                expandReturnElements = array_append(expandReturnElements, retElem);
            } else {
                TrieMapIterator *it = TrieMap_Iterate(schema->attributes, "", 0);
                while(TrieMapIterator_Next(it, &prop, &prop_len, &ptr)) {
                    prop_len = MIN(255, prop_len);
                    memcpy(buffer, prop, prop_len);
                    buffer[prop_len] = '\0';

                    /* Create a new return element foreach property. */
                    expanded_exp = AR_EXP_NewVariableOperandNode(ast, collapsed_entity->alias, buffer);
                    retElem = _NewReturnElementNode(elem->alias, expanded_exp);
                    expandReturnElements = array_append(expandReturnElements, retElem);
                }
                TrieMapIterator_Free(it);
            }
        } else {
            expandReturnElements = array_append(expandReturnElements, elem);
        }
    }

    /* Override previous return clause. */
    array_free(ast->return_expressions);
    ast->return_expressions = expandReturnElements;
}

AST* ParseQuery(const char *query, size_t qLen, char **errMsg) {
    return Query_Parse(query, qLen, errMsg);
}

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const cypher_astnode_t *ast) {
    char *reason;
    int ast_count = array_len(ast);
    for(int i = 0; i < ast_count; i++) {
        if (AST_Validate(ast[i], &reason) != AST_VALID) {
            RedisModule_ReplyWithError(ctx, reason);
            free(reason);
            return AST_INVALID;
        }
    }
    
    /* For the timebeing we do not allow re-definition of identifiers
     * for example:
     * MATCH (p) WITH max(p.v) AS maximum MATCH (p) RETURN p 
     * TODO: lift this restriction. */
    char *redefined_identifier = NULL;
    if(ast_count > 1) {
        TrieMap *global_identifiers = AST_Identifiers(ast[0]);
        for(int i = 1; i < ast_count; i++) {
            TrieMap *local_identifiers = AST_Identifiers(ast[i]);
            TrieMapIterator *it = TrieMap_Iterate(local_identifiers, "", 0);
            void *v;
            tm_len_t len;
            char *identifier;
            while(TrieMapIterator_Next(it, &identifier, &len, &v)) {
                if(!TrieMap_Add(global_identifiers, identifier, len, NULL, TrieMap_DONT_CARE_REPLACE)) {
                    redefined_identifier = rm_malloc(sizeof(char) * (len+1));
                    memcpy(redefined_identifier, identifier, len);
                    redefined_identifier[len] = '\0';
                    break;
                }
            }
            TrieMapIterator_Free(it);
            TrieMap_Free(local_identifiers, TrieMap_NOP_CB);
            if(redefined_identifier) break;
        }
        TrieMap_Free(global_identifiers, TrieMap_NOP_CB);

        if(redefined_identifier) {
            asprintf(&reason, "Identifier '%s' defined more than once", redefined_identifier);
            RedisModule_ReplyWithError(ctx, reason);
            rm_free(redefined_identifier);
            free(reason);
            return AST_INVALID;
        }
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

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const cypher_parse_result_t *ast) {
    char *reason;
    AST_Validation res = NEWAST_Validate(ast, &reason);
    if (res != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        free(reason);
        return AST_INVALID;
    }

	Vector_Free(ast->matchNode->_mergedPatterns);
	Vector_Free(ast->matchNode->patterns);
	free(ast->matchNode);

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

void _ReturnExpandAll(NEWAST *ast) {
    unsigned int identifier_count = ast->identifier_map->cardinality;
    ast->return_expressions = array_new(AR_ExpNode*, identifier_count);

    // TrieMapIterator *it = TrieMap_Iterate(ast->identifier_map, "", 0);
    // char *ptr;
    // tm_len_t len;
    // unsigned int *id;

    // while (TrieMapIterator_Next(it, &ptr, &len, (void**)&id)) {
        // NEWAST_GraphEntity *entity = NEWAST_GetEntity(ast, *id);
    for (unsigned int i = 0; i < identifier_count; i ++) {
        NEWAST_GraphEntity *entity = ast->defined_entities[i];
        AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(ast, entity->alias, NULL);

        ReturnElementNode *ret = _NewReturnElementNode(entity->alias, exp);
        ast->return_expressions = array_append(ast->return_expressions, ret);
    }
}

void _BuildReturnExpressions(NEWAST *ast) {
    // Handle RETURN entities
    const cypher_astnode_t *ret_clause = NEWAST_GetClause(ast->root, CYPHER_AST_RETURN);
    if (!ret_clause) return;

    // Query is of type "RETURN *",
    // collect all defined identifiers and create return elements for them
    if (cypher_ast_return_has_include_existing(ret_clause)) return _ReturnExpandAll(ast);

    unsigned int count = cypher_ast_return_nprojections(ret_clause);
    ast->return_expressions = array_new(AR_ExpNode*, count);
    for (unsigned int i = 0; i < count; i++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
        const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);
        AR_ExpNode *exp = AR_EXP_FromExpression(ast, expr);
        // If projection is aliased, use the aliased name in the arithmetic expression
        const char *alias = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        // Note - doesn't just capture AS aliases, but things like 'e.name'
        // from RETURN e.name
        if (alias_node) {
            alias = cypher_ast_identifier_get_name(alias_node);
        } else if (cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
            alias = cypher_ast_identifier_get_name(expr);
        } else {
            assert(false);
        }
        ast->return_expressions = array_append(ast->return_expressions, _NewReturnElementNode(alias, exp));
        // ast->return_expressions = array_append(ast->return_expressions, New_AST_Entity(alias, A_EXPRESSION, exp));
    }

    // Handle ORDER entities
    const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(ret_clause);
    if (!order_clause) return;

    count = cypher_ast_order_by_nitems(order_clause);
    ast->order_expressions = rm_malloc(count * sizeof(AR_ExpNode*));
    ast->order_expression_count = count;
    for (unsigned int i = 0; i < count; i++) {
        // Returns CYPHER_AST_SORT_ITEM types
        // TODO write a libcypher PR to correct the documentation on this.
        const cypher_astnode_t *order_item = cypher_ast_order_by_get_item(order_clause, i);
        const cypher_astnode_t *expr = cypher_ast_sort_item_get_expression(order_item);
        ast->order_expressions[i] = AR_EXP_FromExpression(ast, expr);
    }
}

// TODO maybe put in resultset.c? _buildExpressions repetition from project and aggregate
/*
void _prepareResultset(NEWAST *ast) {
    // Compute projected record length:
    // Number of returned expressions + number of order-by expressions.
    ExpandCollapsedNodes(ast);
    ResultSet_CreateHeader(op->resultset);

    // const NEWAST *ast = NEWAST_GetFromLTS();

    op->orderByExpCount = ast->order_expression_count;
    op->returnExpCount = ast->return_expression_count;

    op->expressions = rm_malloc((op->orderByExpCount + op->returnExpCount) * sizeof(AR_ExpNode*));

    // TODO redundancy; already performed by ResultSet header creation
    // Compose RETURN clause expressions.
    for(uint i = 0; i < op->returnExpCount; i++) {
        // TODO weird?
        op->expressions[i] = ast->return_expressions[i];
    }

    // Compose ORDER BY expressions.
    for(uint i = 0; i < op->orderByExpCount; i++) {
        op->expressions[i + op->orderByExpCount] = ast->order_expressions[i];
    }

}
*/

void ModifyAST(GraphContext *gc, AST *ast, NEWAST *new_ast) {
    if(ast->matchNode) _AST_optimize_traversal_direction(ast);

    if(ast->mergeNode) {
        /* Create match clause which will try to match
         * against pattern specified within merge clause. */
        _replicateMergeClauseToMatchClause(ast);
    }

    AST_NameAnonymousNodes(ast);
    _inlineProperties(ast);

    NEWAST_BuildAliasMap(new_ast);

    _BuildReturnExpressions(new_ast);
    if(NEWAST_ReturnClause_ContainsCollapsedNodes(new_ast->root) == 1) {
        /* Expand collapsed nodes. */
        ExpandCollapsedNodes(new_ast);
    }
}

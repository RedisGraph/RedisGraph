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

            AST_FilterNode *filterNode = New_AST_PredicateNode(lhs, OP_EQUAL, rhs);

            /* Create WHERE clause if missing. */
            if(ast->whereNode == NULL) {
                ast->whereNode = New_AST_WhereNode(filterNode);
            } else {
                /* Introduce filter with AND operation. */
                AST_FilterNode *left = ast->whereNode->filters;
                AST_FilterNode *right = filterNode;
                ast->whereNode->filters = New_AST_ConditionNode(left, OP_AND, right);
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

void ExpandCollapsedNodes(NEWAST *ast) {

    char buffer[256];
    GraphContext *gc = GraphContext_GetFromTLS();

    unsigned int return_expression_count = array_len(ast->return_expressions);
    ReturnElementNode **expandReturnElements = array_new(ReturnElementNode*, return_expression_count);

    /* Scan return clause, search for collapsed nodes. */
    for (unsigned int i = 0; i < return_expression_count; i++) {
        ReturnElementNode *elem = ast->return_expressions[i];
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
            // Use the user-provided alias if one is provided
            char *alias = elem->alias ? (char*)elem->alias : exp->operand.variadic.entity_alias;
            // char *alias = exp->operand.variadic.entity_alias;
            unsigned int id = NEWAST_GetAliasID(ast, alias);
            AR_ExpNode *collapsed_entity = NEWAST_GetEntity(ast, id);
            // Entity was an expression rather than a node or edge
            // if (collapsed_entity->t != A_ENTITY) continue;

            const cypher_astnode_t *ast_entity = collapsed_entity->operand.variadic.ast_ref;

            cypher_astnode_type_t type = cypher_astnode_type(ast_entity);

            SchemaType schema_type;
            Schema *schema;
            const char *label = NULL;
            if (type == CYPHER_AST_NODE_PATTERN) {
                schema_type = SCHEMA_NODE;
                if (cypher_ast_node_pattern_nlabels(ast_entity) > 0) {
                    const cypher_astnode_t *label_node = cypher_ast_node_pattern_get_label(ast_entity, 0);
                    label = cypher_ast_label_get_name(label_node);
                }
            } else if (type == CYPHER_AST_REL_PATTERN) {
                schema_type = SCHEMA_EDGE;
                if (cypher_ast_rel_pattern_nreltypes(ast_entity) > 0) {
                    // TODO collect all reltypes or update logic elesewhere
                    const cypher_astnode_t *reltype_node = cypher_ast_rel_pattern_get_reltype(ast_entity, 0);
                    label = cypher_ast_reltype_get_name(reltype_node);
                }
            } else if (type == CYPHER_AST_UNWIND) {
                // For UNWIND clauses, use the collection alias as the expression
                const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(ast_entity);
                const char *alias = cypher_ast_identifier_get_name(alias_node);
                AR_ExpNode *exp = AR_EXP_NewConstOperandNode(SI_ConstStringVal((char*)alias));
                ReturnElementNode *unwindElem = _NewReturnElementNode(elem->alias, exp);
                expandReturnElements = array_append(expandReturnElements, elem);
                continue;
            } else {
                assert(false);
            }

            /* Find label's properties. */
            if(label) {
                /* Collapsed entity has a label. */
                schema = GraphContext_GetSchema(gc, label, schema_type);
            } else {
                /* Entity does have a label, Consult with unified schema. */
                schema = GraphContext_GetUnifiedSchema(gc, schema_type);
            }

            void *ptr = NULL;       /* schema property value, (not in use). */
            char *prop = NULL;      /* Entity property. */
            tm_len_t prop_len = 0;  /* Length of entity's property. */

            AR_ExpNode *expanded_exp;
            ReturnElementNode *retElem;
            if(!schema || Schema_AttributeCount(schema) == 0) {
                /* Schema missing or
                 * label doesn't have any properties.
                 * Create a fake return element. */
                expanded_exp = AR_EXP_NewConstOperandNode(SI_ConstStringVal(""));
                // Incase an alias is given use it, otherwise use the variable name.
                retElem = _NewReturnElementNode(elem->alias, expanded_exp);
                expandReturnElements = array_append(expandReturnElements, retElem);
            } else {
                TrieMapIterator *it = TrieMap_Iterate(schema->attributes, "", 0);
                while(TrieMapIterator_Next(it, &prop, &prop_len, &ptr)) {
                    prop_len = MIN(255, prop_len);
                    memcpy(buffer, prop, prop_len);
                    buffer[prop_len] = '\0';

                    // TODO validate and simplify
                    /* Create a new return element foreach property. */
                    expanded_exp = AR_EXP_NewVariableOperandNode(ast, ast_entity, collapsed_entity->operand.variadic.entity_alias, buffer);
                    // expanded_exp = AR_EXP_NewVariableOperandNode(ast, alias, buffer);
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

AST** ParseQuery(const char *query, size_t qLen, char **errMsg) {
    AST **asts = Query_Parse(query, qLen, errMsg);
    if(asts) {
        for(int i = 0; i < array_len(asts); i++) {
            /* Create match clause which will try to match against pattern specified within merge clause. */
            if(asts[i]->mergeNode) _replicateMergeClauseToMatchClause(asts[i]);

            AST_NameAnonymousNodes(asts[i]);
            // Mark each alias with a unique ID.
            AST_WithNode *withClause = (i > 0) ? asts[i-1]->withNode : NULL;
            AST_MapAliasToID(asts[i], withClause);
        }
    }
    return asts;
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

    for (unsigned int i = 0; i < identifier_count; i ++) {
        AR_ExpNode *entity = ast->defined_entities[i];
        // TODO Temporary, make better solution
        if (!strncmp(entity->operand.variadic.entity_alias, "anon_", 5)) continue;
        ReturnElementNode *ret = _NewReturnElementNode(NULL, entity);
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
        if (alias_node) {
            alias = cypher_ast_identifier_get_name(alias_node);
            // TODO standardize logic (make a separate routine for this, can drop ID pointer elsewhere

            // TODO expressions like TYPE(e) have aliases, so can get in this block?
            // Kludge for testing; improve when possible
            void *v = TrieMap_Find(ast->identifier_map, (char*)alias, strlen(alias));
            if (v == TRIEMAP_NOTFOUND) {
                unsigned int *entityID = malloc(sizeof(unsigned int));
                *entityID = ast->identifier_map->cardinality;
                TrieMap_Add(ast->identifier_map, (char*)alias, strlen(alias), entityID, TrieMap_DONT_CARE_REPLACE);
            }
        } else if (cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
            // TODO don't need to do anything, delete this section once validated
            // alias = cypher_ast_identifier_get_name(expr);
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

// TODO
// TODO maybe put in resultset.c? _buildExpressions repetition from project and aggregate
/*
void _prepareResultset(NEWAST *ast) {
    // Compute projected record length:
    // Number of returned expressions + number of order-by expressions.
    ExpandCollapsedNodes(ast);
    ResultSet_CreateHeader(op->resultset);

    // const NEWAST *ast = NEWAST_GetFromTLS();

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

void ModifyAST(GraphContext *gc, AST **ast, NEWAST *new_ast) {
    for(int i = 0; i < array_len(ast); i++) {
        if(ast[i]->matchNode) _AST_optimize_traversal_direction(ast[i]);
        _inlineProperties(ast[i]);
    }

    NEWAST_BuildAliasMap(new_ast);

    _BuildReturnExpressions(new_ast);
    if(NEWAST_ReturnClause_ContainsCollapsedNodes(new_ast->root) == 1) {
        /* Expand collapsed nodes. */
        ExpandCollapsedNodes(new_ast);
    }
}

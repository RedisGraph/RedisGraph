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
#include "arithmetic/agg_ctx.h"
#include "arithmetic/repository.h"
#include "../deps/libcypher-parser/lib/src/cypher-parser.h"

void ExpandCollapsedNodes(AST *ast) {

    char buffer[256];
    GraphContext *gc = GraphContext_GetFromTLS();

    unsigned int return_expression_count = array_len(ast->return_expressions);
    const char **expandReturnElements = array_new(const char*, return_expression_count);

    /* Scan return clause, search for collapsed nodes. */
    for (unsigned int i = 0; i < return_expression_count; i++) {
        const char *column_name = ast->return_expressions[i];
        AR_ExpNode *exp = AST_GetEntityFromAlias(ast, (char*)column_name);

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
            const cypher_astnode_t *ast_entity = exp->operand.variadic.ast_ref;
            AR_ExpNode *collapsed_entity = ast->defined_entities[exp->operand.variadic.entity_alias_idx];
            // Entity was an expression rather than a node or edge
            // if (collapsed_entity->t != A_ENTITY) continue;


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
                expandReturnElements = array_append(expandReturnElements, column_name);
                continue;
            } else if (type == CYPHER_AST_IDENTIFIER) {
                // Observed in query "UNWIND [1,2,3] AS a RETURN a AS e"
                const char *alias = cypher_ast_identifier_get_name(ast_entity);
                expandReturnElements = array_append(expandReturnElements, column_name);
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
            if(!schema || Schema_AttributeCount(schema) == 0) {
                /* Schema missing or
                 * label doesn't have any properties.
                 * Create a fake return element. */
                expanded_exp = AR_EXP_NewConstOperandNode(SI_ConstStringVal(""));
                // Incase an alias is given use it, otherwise use the variable name.
                expandReturnElements = array_append(expandReturnElements, column_name);
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
                    unsigned int id = AST_AddRecordEntry(ast);
                    AR_EXP_AssignRecordIndex(expanded_exp, id);
                    ast->defined_entities = array_append(ast->defined_entities, expanded_exp);

                    // TODO memory leak (and ugly), but only required until we don't expand collapsed entities
                    char *expanded_name;
                    AR_EXP_ToString(expanded_exp, &expanded_name);
                    AST_MapAlias(ast, expanded_name, expanded_exp);
                    // expandReturnElements = array_append(expandReturnElements, column_name);
                    expandReturnElements = array_append(expandReturnElements, expanded_name);
                }
                TrieMapIterator_Free(it);
            }
        } else {
            expandReturnElements = array_append(expandReturnElements, column_name);
        }
    }

    /* Override previous return clause. */
    array_free(ast->return_expressions);
    ast->return_expressions = expandReturnElements;
}

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const AST *ast) {
    char *reason;
    AST_Validation res = AST_Validate(ast, &reason);
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
// static bool _AST_should_reverse_pattern(Vector *pattern) {
    // int transposed = 0; // Number of transposed edges
    // int edge_count = 0; // Total number of edges.
    // int pattern_length = Vector_Size(pattern);

    // // Count how many edges are going right to left.
    // for(int i = 0; i < pattern_length; i++) {
        // AST_GraphEntity *match_element;
        // Vector_Get(pattern, i, &match_element);

        // if(match_element->t != N_LINK) continue;

        // edge_count++;
        // AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        // if(edge->direction ==  N_RIGHT_TO_LEFT) transposed++;
    // }

    // // No edges.
    // if(edge_count == 0) return false;
    // return (transposed > edge_count/2);
// }

/* Construct a new MATCH clause by cloning the current one
 * and reversing traversal patterns to reduce matrix transpose
 * operation. */
// TODO re-implement this
// static void _AST_reverse_match_patterns(AST *ast) {
    // size_t pattern_count = Vector_Size(ast->matchNode->patterns);
    // Vector *patterns = NewVector(Vector*, pattern_count);

    // for(int i = 0; i < pattern_count; i++) {
        // Vector *pattern;
        // Vector_Get(ast->matchNode->patterns, i, &pattern);

        // size_t pattern_length = Vector_Size(pattern);
        // Vector *v = NewVector(AST_GraphEntity*, pattern_length);

        // if(!_AST_should_reverse_pattern(pattern)) {
            // // No need to reverse, simply clone pattern.
            // for(int j = 0; j < pattern_length; j++) {
                // AST_GraphEntity *e;
                // Vector_Get(pattern, j, &e);
                // e = Clone_AST_GraphEntity(e);
                // Vector_Push(v, e);
            // }
        // }
        // else {
            /* Reverse pattern:
             * Create a new pattern where edges been reversed.
             * Nodes should be introduced in reverse order:
             * (C)<-[B]-(A)
             * (A)-[B]->(C) */
            // for(int j = pattern_length-1; j >= 0; j--) {
                // AST_GraphEntity *e;
                // Vector_Get(pattern, j, &e);
                // e = Clone_AST_GraphEntity(e);

                // if(e->t == N_LINK) {
                    // AST_LinkEntity *l = (AST_LinkEntity*)e;
                    // // Reverse pattern.
                    // if(l->direction == N_RIGHT_TO_LEFT) l->direction = N_LEFT_TO_RIGHT;
                    // else l->direction = N_RIGHT_TO_LEFT;
                // }
                // Vector_Push(v, e);
            // }
        // }
        // Vector_Push(patterns, v);
    // }

    // // Free old MATCH clause.
    // Free_AST_MatchNode(ast->matchNode);
    // // Update AST MATCH clause.
    // ast->matchNode = New_AST_MatchNode(patterns);
// }

// static void _AST_optimize_traversal_direction(AST *ast) {
    /* Inspect each MATCH pattern,
     * see if the number of edges going from right to left ()<-[]-()
     * is greater than the number of edges going from left to right ()-[]->()
     * in which case it's worth reversing the pattern to reduce
     * matrix transpose operations. */

    // bool should_reverse = false;
    // size_t pattern_count = Vector_Size(ast->matchNode->patterns);
    // for(int i = 0; i < pattern_count; i++) {
        // Vector *pattern;
        // Vector_Get(ast->matchNode->patterns, i, &pattern);

        // if(_AST_should_reverse_pattern(pattern)) {
            // should_reverse = true;
            // break;
        // }
    // }

    // if(should_reverse) _AST_reverse_match_patterns(ast);
// }

void _ReturnExpandAll(AST *ast) {
    unsigned int identifier_count = array_len(ast->defined_entities);
    ast->return_expressions = array_new(AR_ExpNode*, identifier_count);

    for (unsigned int i = 0; i < identifier_count; i ++) {
        AR_ExpNode *entity = ast->defined_entities[i];
        char *alias = entity->operand.variadic.entity_alias;
        if (alias) {
            ast->return_expressions = array_append(ast->return_expressions, alias);
        }
    }
}

void _BuildReturnExpressions(AST *ast) {
    // Handle RETURN entities
    const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
    if (!ret_clause) return;

    // Query is of type "RETURN *",
    // collect all defined identifiers and create return elements for them
    if (cypher_ast_return_has_include_existing(ret_clause)) return _ReturnExpandAll(ast);

    unsigned int count = cypher_ast_return_nprojections(ret_clause);
    ast->return_expressions = array_new(AR_ExpNode*, count);
    for (unsigned int i = 0; i < count; i++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
        const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);

        AR_ExpNode *exp = NULL;
        char *identifier = NULL;

        if (cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
            // Retrieve "a" from "RETURN a" or "RETURN a AS e"
            identifier = (char*)cypher_ast_identifier_get_name(expr);
            exp = AST_GetEntityFromAlias(ast, (char*)identifier);
        }

        if (exp == NULL) {
            // Identifier did not appear in previous clauses.
            // It may be a constant or a function call (or other?)
            // Create a new entity to represent it.
            exp = AR_EXP_FromExpression(ast, expr);

            // Make space for entity in record
            unsigned int id = AST_AddRecordEntry(ast);
            AR_EXP_AssignRecordIndex(exp, id);
            // Add entity to the set of entities to be populated
            ast->defined_entities = array_append(ast->defined_entities, exp);
        }

        // If the projection is aliased, add the alias to mappings and Record
        char *alias = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node) {
            // The projection either has an alias (AS) or is a function call.
            alias = (char*)cypher_ast_identifier_get_name(alias_node);
            // TODO can the alias have appeared in an earlier clause?

            // Associate alias with the expression

            // TODO This seems like more work than should be necessary
            // Make node for alias
            AR_ExpNode *alias_exp = AR_EXP_FromExpression(ast, expr);

            // Make space for alias entity in record
            unsigned int id = AST_AddRecordEntry(ast);
            AR_EXP_AssignRecordIndex(alias_exp, id);
            // Add entity to the set of entities to be populated
            ast->defined_entities = array_append(ast->defined_entities, alias_exp);
            AST_MapAlias(ast, alias, alias_exp);
        }

        const char *column_name = alias ? alias : exp->operand.variadic.entity_alias;
        ast->return_expressions = array_append(ast->return_expressions, column_name);
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

void ModifyAST(GraphContext *gc, AST *ast) {
    // for(int i = 0; i < array_len(ast); i++) {
        // if(ast[i]->matchNode) _AST_optimize_traversal_direction(ast[i]);
        // _inlineProperties(ast[i]);
    // }

    assert(AST_GetClause(ast, CYPHER_AST_WITH) == NULL);
    AST_BuildAliasMap(ast);

    _BuildReturnExpressions(ast);
    if(AST_ReturnClause_ContainsCollapsedNodes(ast)) {
        /* Expand collapsed nodes. */
        ExpandCollapsedNodes(ast);
    }
}

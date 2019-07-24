/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/rmalloc.h"
#include "../util/arr.h"
#include "../util/vector.h"
#include "../util/qsort.h"
#include "../graph/entities/edge.h"
#include "./optimizations/optimizer.h"
#include "./optimizations/optimizations.h"
#include "../arithmetic/algebraic_expression.h"
#include "../ast/ast_build_op_contexts.h"
#include "../ast/ast_build_filter_tree.h"

// TODO dup of work in ExecutionPlanInit, but we need record_map associated with
// ops before calling optimizePlan
static void _associateRecordMap(OpBase *root, RecordMap *record_map) {
    // If this op has already been initialized,
    // we don't need to recurse further.
    if (root->record_map != NULL) return;

    // Share this ExecutionPlanSegment's record map with the operation.
    root->record_map = record_map;

    for(int i = 0; i < root->childCount; i++) {
        _associateRecordMap(root->children[i], record_map);
    }
}

/* Keep track of resolved variables.
 * Add variables modified/set by op to resolved. */
static void _UpdateResolvedVariables(rax *resolved, OpBase *op) {
    assert(resolved && op);
    if(!op->modifies) return;

    uint count = array_len(op->modifies);
    for(uint i = 0; i < count; i++) {
        uint modified_id = op->modifies[i];
        raxInsert(resolved, (unsigned char*)&modified_id, sizeof(modified_id), NULL, NULL);
    }
}

AR_ExpNode** _ReturnExpandAll(RecordMap *record_map) {
    AST *ast = AST_GetFromTLS();

    // Collect all unique aliases
    const char **aliases = AST_CollectAliases(ast);
    uint count = array_len(aliases);

    // Build an expression for each alias
    AR_ExpNode **return_expressions = array_new(AR_ExpNode*, count);
    for (int i = 0; i < count; i ++) {
        AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(record_map, aliases[i], NULL);
        exp->resolved_name = aliases[i];
        return_expressions = array_append(return_expressions, exp);
    }

    array_free(aliases);
    return return_expressions;
}

// Handle ORDER entities
AR_ExpNode** _BuildOrderExpressions(RecordMap *record_map, AR_ExpNode **projections, const cypher_astnode_t *order_clause) {
    bool ascending = true;

    uint projection_count = array_len(projections);
    uint count = cypher_ast_order_by_nitems(order_clause);
    AR_ExpNode **order_exps = array_new(AR_ExpNode*, count);

    for (uint i = 0; i < count; i++) {
        const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
        const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
        /* TODO need to think about logic here - can introduce new data, reference
         * projections, reference otherwise-unprojected aliases. In the referencing-projection
         * case, we may not be allowed to use the pre-existing record index:
         * RETURN e.name as v ORDER BY v */
        AR_ExpNode *exp = NULL;
        if (cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER) {
            // Order expression is a reference to an alias in the query
            const char *alias = cypher_ast_identifier_get_name(ast_exp);
            for (uint j = 0; j < projection_count; j ++) {
                AR_ExpNode *projection = projections[j];
                if (!strcmp(projection->resolved_name, alias)) {
                    // The projection must be cloned to avoid a double free
                    exp = AR_EXP_Clone(projection);
                    break;
                }
            }
        } else {
            // Independent operator like:
            // ORDER BY COUNT(a)
            exp = AR_EXP_FromExpression(record_map, ast_exp);
        }
        // AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

        order_exps = array_append(order_exps, exp);
        // TODO direction should be specifiable per order entity
        ascending = cypher_ast_sort_item_is_ascending(item);
    }

    // *direction = ascending ? DIR_ASC : DIR_DESC;

    return order_exps;
}

// Handle RETURN entities
AR_ExpNode** _BuildReturnExpressions(RecordMap *record_map, const cypher_astnode_t *ret_clause) {
    // Query is of type "RETURN *",
    // collect all defined identifiers and create return elements for them
    if (cypher_ast_return_has_include_existing(ret_clause)) return _ReturnExpandAll(record_map);

    uint count = cypher_ast_return_nprojections(ret_clause);
    AR_ExpNode **return_expressions = array_new(AR_ExpNode*, count);
    for (uint i = 0; i < count; i++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
        // The AST expression can be an identifier, function call, or constant
        const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

        // Construction an AR_ExpNode to represent this return entity.
        AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);


        // Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
        // the entity.prop combination ("a.val"), or the function call ("MAX(a.val)")
        const char *identifier = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node) {
            // The projection either has an alias (AS), is a function call, or is a property specification (e.name).
            identifier = cypher_ast_identifier_get_name(alias_node);
        } else {
            // This expression did not have an alias, so it must be an identifier
            const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);
            assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
            // Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
            identifier = cypher_ast_identifier_get_name(ast_exp);
        }

        exp->resolved_name = identifier;

        return_expressions = array_append(return_expressions, exp);
    }

    return return_expressions;
}

AR_ExpNode** _BuildWithExpressions(RecordMap *record_map, const cypher_astnode_t *with_clause) {
    uint count = cypher_ast_with_nprojections(with_clause);
    AR_ExpNode **with_expressions = array_new(AR_ExpNode*, count);
    for (uint i = 0; i < count; i++) {
        const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
        const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

        // Construction an AR_ExpNode to represent this entity.
        AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

        // Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
        // the entity.prop combination ("a.val"), or the function call ("MAX(a.val)").
        // The WITH clause requires that the resolved name be an alias or identifier.
        const char *identifier = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node) {
            // The projection either has an alias (AS), is a function call, or is a property specification (e.name).
            /// TODO should issue syntax failure in the latter 2 cases
            identifier = cypher_ast_identifier_get_name(alias_node);
        } else {
            // This expression did not have an alias, so it must be an identifier
            const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);
            assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
            // Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
            identifier = cypher_ast_identifier_get_name(ast_exp);
        }

        exp->resolved_name = identifier;

        with_expressions = array_append(with_expressions, exp);
    }

    return with_expressions;

}

AR_ExpNode** _BuildCallProjections(RecordMap *record_map, const cypher_astnode_t *call_clause) {
    // Handle yield entities
    uint yield_count = cypher_ast_call_nprojections(call_clause);
    AR_ExpNode **expressions = array_new(AR_ExpNode*, yield_count);

    for (uint i = 0; i < yield_count; i ++) {
        const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
        const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

        // Construction an AR_ExpNode to represent this entity.
        AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

        const char *identifier = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node) {
            // The projection either has an alias (AS), is a function call, or is a property specification (e.name).
            identifier = cypher_ast_identifier_get_name(alias_node);
        } else {
            // This expression did not have an alias, so it must be an identifier
            const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);
            assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
            // Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
            identifier = cypher_ast_identifier_get_name(ast_exp);
        }

        exp->resolved_name = identifier;

        expressions = array_append(expressions, exp);
    }

    // If the procedure call is missing its yield part, include procedure outputs. 
    if (yield_count == 0) {
        const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
        ProcedureCtx *proc = Proc_Get(proc_name);
        assert(proc);

        unsigned int output_count = array_len(proc->output);
        for (uint i = 0; i < output_count; i++) {
            const char *name = proc->output[i]->name;

            // TODO the 'name' variable doesn't have an AST ID, so an assertion in
            // AR_EXP_NewVariableOperandNode() fails without this call. Consider options.
            ASTMap_FindOrAddAlias(AST_GetFromTLS(), name, IDENTIFIER_NOT_FOUND);
            AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(record_map, name, NULL);
            exp->resolved_name = name; // TODO kludge?
            expressions = array_append(expressions, exp);
        }
    }

    return expressions;
}

const char** _BuildCallArguments(RecordMap *record_map, const cypher_astnode_t *call_clause) {
    // Handle argument entities
    uint arg_count = cypher_ast_call_narguments(call_clause);
    // if (expressions == NULL) expressions = array_new(AR_ExpNode*, arg_count);
    const char **arguments = array_new(const char*, arg_count);
    for (uint i = 0; i < arg_count; i ++) {

        const cypher_astnode_t *ast_exp = cypher_ast_call_get_argument(call_clause, i);

        const cypher_astnode_t *identifier_node = cypher_ast_projection_get_alias(ast_exp);
        const char *identifier = cypher_ast_identifier_get_name(identifier_node);

        arguments = array_append(arguments, identifier);
    }

    return arguments;
}

void _ExecutionPlanSegment_ProcessQueryGraph(ExecutionPlanSegment *segment, QueryGraph *qg, FT_FilterNode *ft, rax *resolved, Vector *ops) {
    GraphContext *gc = GraphContext_GetFromTLS();
    AST *ast = AST_GetFromTLS();

    QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
    uint connectedComponentsCount = array_len(connectedComponents);
    segment->connected_components = connectedComponents;

    /* For every connected component.
     * Incase we're dealing with multiple components
     * we'll simply join them all together with a join operation. */
    OpBase *cartesianProduct = NULL;
    if(connectedComponentsCount > 1) {
        cartesianProduct = NewCartesianProductOp();
        Vector_Push(ops, cartesianProduct);
    }

    // Keep track after all traversal operations along a pattern.
    Vector *traversals = NewVector(OpBase*, 1);
    OpBase *op;

    for(uint i = 0; i < connectedComponentsCount; i++) {
        QueryGraph *cc = connectedComponents[i];
        uint edge_count = array_len(cc->edges);
        if(edge_count == 0) {
            /* Node scan. */
            QGNode *n = cc->nodes[0];
            uint rec_idx = RecordMap_FindOrAddID(segment->record_map, n->id);
            if(n->labelID != GRAPH_NO_LABEL) op = NewNodeByLabelScanOp(n, rec_idx);
            else op = NewAllNodeScanOp(gc->g, n, rec_idx);
            Vector_Push(traversals, op);
            _UpdateResolvedVariables(resolved, op);
        } else {
            size_t expCount = 0;
            AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc, segment->record_map, &expCount);

            // Reorder exps, to the most performant arrangement of evaluation.
            orderExpressions(exps, expCount, segment->record_map, ft);

            AlgebraicExpression *exp = exps[0];
            selectEntryPoint(exp, segment->record_map, ft);

            // Retrieve the AST ID for the source node
            uint ast_id = exp->src_node->id;
            // Convert to a Record ID
            uint record_id = RecordMap_FindOrAddID(segment->record_map, ast_id);

            // Create SCAN operation.
            if(exp->src_node->label) {
                /* Resolve source node by performing label scan,
                 * in which case if the first algebraic expression operand
                 * is a label matrix (diagonal) remove it, otherwise
                 * the label matrix associated with source's label is located
                 * within another traversal operation, for the timebeing do not
                 * try to locate and remove it, there's no real harm except some performace hit
                 * in keeping that label matrix. */
                if(exp->operands[0].diagonal) AlgebraicExpression_RemoveTerm(exp, 0, NULL);
                op = NewNodeByLabelScanOp(exp->src_node, record_id);
            } else {
                op = NewAllNodeScanOp(gc->g, exp->src_node, record_id);
            }

            Vector_Push(traversals, op);
            _UpdateResolvedVariables(resolved, op);

            for(int i = 0; i < expCount; i++) {
                exp = exps[i];
                if(exp->operand_count == 0) continue;

                uint ast_id;
                uint src_node_idx;
                uint dest_node_idx;
                uint edge_idx = IDENTIFIER_NOT_FOUND;

                // Make sure that all entities are represented in Record
                ast_id = exp->src_node->id;
                src_node_idx = RecordMap_FindOrAddID(segment->record_map, ast_id);

                ast_id = exp->dest_node->id;
                dest_node_idx = RecordMap_FindOrAddID(segment->record_map, ast_id);

                if (exp->edge) {
                    ast_id = exp->edge->id;
                    edge_idx = RecordMap_FindOrAddID(segment->record_map, ast_id);
                }

                if(exp->edge && QGEdge_VariableLength(exp->edge)) {
                    op = NewCondVarLenTraverseOp(exp,
                            exp->edge->minHops,
                            exp->edge->maxHops,
                            src_node_idx,
                            dest_node_idx,
                            gc->g);
                } else {
                    op = NewCondTraverseOp(gc->g, exp, src_node_idx, dest_node_idx, edge_idx, TraverseRecordCap(ast));
                }

                Vector_Push(traversals, op);
                _UpdateResolvedVariables(resolved, op);
            }

            // Free the expressions array, as its parts have been converted into operations
            rm_free(exps);
        }

        if(connectedComponentsCount > 1) {
            // Connect traversal operations.
            OpBase *childOp;
            OpBase *parentOp;
            Vector_Pop(traversals, &parentOp);
            // Connect cartesian product to the root of traversal.
            ExecutionPlan_AddOp(cartesianProduct, parentOp);
            while(Vector_Pop(traversals, &childOp)) {
                ExecutionPlan_AddOp(parentOp, childOp);
                parentOp = childOp;
            }
        } else {
            for(int traversalIdx = 0; traversalIdx < Vector_Size(traversals); traversalIdx++) {
                Vector_Get(traversals, traversalIdx, &op);
                Vector_Push(ops, op);
            }
        }
        Vector_Clear(traversals);
    }

    Vector_Free(traversals);
}

// Map the required AST entities and build expressions to match
// the AST slice's WITH, RETURN, and ORDER clauses
void _ExecutionPlanSegment_BuildProjections(ExecutionPlanSegment *segment, AST *ast) {
    // Retrieve a RETURN clause if one is specified in this AST's range
    const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
    // Retrieve a WITH clause if one is specified in this AST's range
    const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
    // We cannot have both a RETURN and WITH clause
    assert(!(ret_clause && with_clause));
    segment->projections = NULL;
    segment->order_expressions = NULL;

    const cypher_astnode_t *order_clause = NULL;
    if (ret_clause) {
        segment->projections = _BuildReturnExpressions(segment->record_map, ret_clause);
        order_clause = cypher_ast_return_get_order_by(ret_clause);
    } else if (with_clause) {
        segment->projections = _BuildWithExpressions(segment->record_map, with_clause);
        order_clause = cypher_ast_with_get_order_by(with_clause);
    }

    if (order_clause) segment->order_expressions = _BuildOrderExpressions(segment->record_map, segment->projections, order_clause);

    // const cypher_astnode_t *call_clause = AST_GetClause(ast, CYPHER_AST_CALL);
    // if(call_clause) {
        // segment->projections = _BuildCallExpressions(segment->record_map, segment->projections, call_clause);
    // }

}

// Map the AST entities described in SET and DELETE clauses.
// This is necessary so that edge references will be constructed prior to forming AlgebraicExpressions.
void _ExecutionPlanSegment_MapReferences(ExecutionPlanSegment *segment, AST *ast) {

    const cypher_astnode_t *set_clause = AST_GetClause(ast, CYPHER_AST_SET);
    if(set_clause) {
        uint nitems = cypher_ast_set_nitems(set_clause);
        for(uint i = 0; i < nitems; i++) {
            const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
            const cypher_astnode_t *key_to_set = cypher_ast_set_property_get_property(set_item); // type == CYPHER_AST_PROPERTY_OPERATOR
            const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(key_to_set);
            assert(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
            const char *alias = cypher_ast_identifier_get_name(prop_expr);
            RecordMap_FindOrAddAlias(segment->record_map, alias);
        }
    }

    const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
    if(delete_clause) {
        uint nitems = cypher_ast_delete_nexpressions(delete_clause);
        for(uint i = 0; i < nitems; i++) {
            const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
            assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
            const char *alias = cypher_ast_identifier_get_name(ast_expr);
            RecordMap_FindOrAddAlias(segment->record_map, alias);
        }
    }
}

ExecutionPlanSegment* _NewExecutionPlanSegment(RedisModuleCtx *ctx, GraphContext *gc, AST *ast, ResultSet *result_set, AR_ExpNode **prev_projections, OpBase *prev_op) {

    // Allocate a new segment
    ExecutionPlanSegment *segment = rm_malloc(sizeof(ExecutionPlanSegment));
    segment->connected_components = NULL;

    // Initialize map of Record IDs
    RecordMap *record_map = RecordMap_New();
    segment->record_map = record_map;

    if (prev_projections) {
        // We have an array of identifiers provided by a prior WITH clause -
        // these will correspond to our first Record entities
        uint projection_count = array_len(prev_projections);
        for (uint i = 0; i < projection_count; i++) {
            AR_ExpNode *projection = prev_projections[i];
            RecordMap_FindOrAddAlias(record_map, projection->resolved_name);
        }
    }

    // Build projections from this AST's WITH, RETURN, and ORDER clauses
    _ExecutionPlanSegment_BuildProjections(segment, ast);

    // Extend the RecordMap to include references from clauses that do not form projections
    // (SET, DELETE)
    _ExecutionPlanSegment_MapReferences(segment, ast);

    Vector *ops = NewVector(OpBase*, 1);

    // Build query graph
    QueryGraph *qg = BuildQueryGraph(gc, ast);
    segment->query_graph = qg;

    // Build filter tree
    FT_FilterNode *filter_tree = AST_BuildFilterTree(ast, record_map, qg);
    segment->filter_tree = filter_tree;

    // Prepare rax for tracking resolved Record IDs
    rax *resolved = raxNew();

    const cypher_astnode_t *call_clause = AST_GetClause(ast, CYPHER_AST_CALL);
    if(call_clause) {
        // A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
        const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
        const char **arguments = _BuildCallArguments(record_map, call_clause);
        AR_ExpNode **yield_exps = _BuildCallProjections(record_map, call_clause);
        uint yield_count = array_len(yield_exps);
        const char **yields = array_new(const char *, yield_count);
        if (segment->projections == NULL) segment->projections = array_new(AR_ExpNode*, yield_count);
        uint *call_modifies = array_new(uint, yield_count);
        for (uint i = 0; i < yield_count; i ++) {
            // TODO revisit this
            // Add yielded expressions to segment projections.
            segment->projections = array_append(segment->projections, yield_exps[i]);
            // Track the names of yielded variables.
            yields = array_append(yields, yield_exps[i]->resolved_name);
            // Track which variables are modified by this operation.
            call_modifies = array_append(call_modifies, yield_exps[i]->operand.variadic.entity_alias_idx);
        }

        OpBase *opProcCall = NewProcCallOp(proc_name, arguments, yields, call_modifies);
        Vector_Push(ops, opProcCall);
    }

    // Build traversal operations for every connected component in the QueryGraph
    if (AST_ContainsClause(ast, CYPHER_AST_MATCH) || AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
        _ExecutionPlanSegment_ProcessQueryGraph(segment, qg, filter_tree, resolved, ops);
    }

    // Set root operation
    const cypher_astnode_t *unwind_clause = AST_GetClause(ast, CYPHER_AST_UNWIND);
    if(unwind_clause) {
        AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(unwind_clause, record_map);

        OpBase *opUnwind = NewUnwindOp(unwind_ast_ctx.record_idx, unwind_ast_ctx.exps);
        Vector_Push(ops, opUnwind);
        _UpdateResolvedVariables(resolved, opUnwind);
    }

    bool create_clause = AST_ContainsClause(ast, CYPHER_AST_CREATE);
    if(create_clause) {
        QueryGraph_AddCreateClauses(gc, ast, qg);
        AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(record_map, ast, qg);
        OpBase *opCreate = NewCreateOp(&result_set->stats,
                                       create_ast_ctx.nodes_to_create,
                                       create_ast_ctx.edges_to_create);
        Vector_Push(ops, opCreate);
        _UpdateResolvedVariables(resolved, opCreate);
    }

    const cypher_astnode_t *merge_clause = AST_GetClause(ast, CYPHER_AST_MERGE);
    if(merge_clause) {
        // A merge clause provides a single path that must exist or be created.
        // As with paths in a MATCH query, build the appropriate traversal operations
        // and append them to the set of ops.
        AST_MergeContext merge_ast_ctx = AST_PrepareMergeOp(record_map, ast, merge_clause, qg);

        // Append a merge operation
        OpBase *opMerge = NewMergeOp(&result_set->stats,
                                     merge_ast_ctx.nodes_to_merge,
                                     merge_ast_ctx.edges_to_merge);
        Vector_Push(ops, opMerge);
        _UpdateResolvedVariables(resolved, opMerge);
    }

    const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
    if(delete_clause) {
        uint *nodes_ref;
        uint *edges_ref;
        AST_PrepareDeleteOp(delete_clause, qg, record_map, &nodes_ref, &edges_ref);
        ResultSetStatistics *stats = (result_set) ? &result_set->stats : NULL;
        OpBase *opDelete = NewDeleteOp(nodes_ref, edges_ref, stats);
        Vector_Push(ops, opDelete);
    }

    const cypher_astnode_t *set_clause = AST_GetClause(ast, CYPHER_AST_SET);
    if(set_clause) {
        // Create a context for each update expression.
        uint nitems;
        EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(set_clause, record_map, &nitems);
        OpBase *op_update = NewUpdateOp(gc, update_exps, nitems, &result_set->stats);
        Vector_Push(ops, op_update);
    }

    const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
    const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);

    assert(!(with_clause && ret_clause));

    uint *modifies = NULL;

    // WITH/RETURN projections have already been constructed from the AST
    AR_ExpNode **projections = segment->projections;

    if (with_clause || ret_clause || call_clause) {
        // TODO improve interface, maybe CollectEntityIDs variant that builds an array
        rax *modifies_ids = raxNew();
        uint exp_count = array_len(projections);
        for (uint i = 0; i < exp_count; i ++) {
            AR_ExpNode *exp = projections[i];
            AR_EXP_CollectEntityIDs(exp, modifies_ids);
        }

        modifies = array_new(uint, raxSize(modifies_ids));
        raxIterator iter;
        raxStart(&iter, modifies_ids);
        raxSeek(&iter, ">=", (unsigned char *)"", 0);
        while (raxNext(&iter)) {
            modifies = array_append(modifies, *(uint*)iter.key);
        }
        raxFree(modifies_ids);
    }

    OpBase *op;

    if(with_clause) {
        // uint *with_projections = AST_WithClauseModifies(ast, with_clause);
        if (AST_ClauseContainsAggregation(with_clause)) {
            op = NewAggregateOp(projections, modifies);
        } else {
            op = NewProjectOp(projections, modifies);
        }
        Vector_Push(ops, op);

        if (cypher_ast_with_is_distinct(with_clause)) {
            op = NewDistinctOp();
            Vector_Push(ops, op);
        }

        const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(with_clause);
        const cypher_astnode_t *limit_clause = cypher_ast_with_get_limit(with_clause);

        uint skip = 0;
        uint limit = 0;
        if (skip_clause) skip = AST_ParseIntegerNode(skip_clause);
        if (limit_clause) limit = AST_ParseIntegerNode(limit_clause);

        if (segment->order_expressions) {
            const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(with_clause);
            int direction = AST_PrepareSortOp(order_clause);
            // The sort operation will obey a specified limit, but must account for skipped records
            uint sort_limit = (limit > 0) ? limit + skip : 0;
            op = NewSortOp(segment->order_expressions, direction, sort_limit);
            Vector_Push(ops, op);
        }

        if (skip_clause) {
            OpBase *op_skip = NewSkipOp(skip);
            Vector_Push(ops, op_skip);
        }

        if (limit_clause) {
            OpBase *op_limit = NewLimitOp(limit);
            Vector_Push(ops, op_limit);
        }
    } else if (ret_clause) {

        // TODO we may not need a new project op if the query is something like:
        // MATCH (a) WITH a.val AS val RETURN val
        // Though we would still need a new projection (barring later optimizations) for:
        // MATCH (a) WITH a.val AS val RETURN val AS e
        if (AST_ClauseContainsAggregation(ret_clause)) {
            op = NewAggregateOp(projections, modifies);
        } else {
            op = NewProjectOp(projections, modifies);
        }
        Vector_Push(ops, op);

        if (cypher_ast_return_is_distinct(ret_clause)) {
            op = NewDistinctOp();
            Vector_Push(ops, op);
        }

        const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(ret_clause);
        const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(ret_clause);
        const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);

        uint skip = 0;
        uint limit = 0;
        if (skip_clause) skip = AST_ParseIntegerNode(skip_clause);
        if (limit_clause) limit = AST_ParseIntegerNode(limit_clause);

        if (segment->order_expressions) {
            int direction = AST_PrepareSortOp(order_clause);
            // The sort operation will obey a specified limit, but must account for skipped records
            uint sort_limit = (limit > 0) ? limit + skip : 0;
            op = NewSortOp(segment->order_expressions, direction, sort_limit);
            Vector_Push(ops, op);
        }

        if (skip_clause) {
            OpBase *op_skip = NewSkipOp(skip);
            Vector_Push(ops, op_skip);
        }

        if (limit_clause) {
            OpBase *op_limit = NewLimitOp(limit);
            Vector_Push(ops, op_limit);
        }

        op = NewResultsOp(result_set, qg);
        Vector_Push(ops, op);
    } else if (call_clause) {
        op = NewResultsOp(result_set, qg);
        Vector_Push(ops, op);
    }

    OpBase *parent_op;
    OpBase *child_op;
    Vector_Pop(ops, &parent_op);
    segment->root = parent_op;

    while(Vector_Pop(ops, &child_op)) {
        ExecutionPlan_AddOp(parent_op, child_op);
        parent_op = child_op;
    }

    Vector_Free(ops);

    if (prev_op) {
        // Need to connect this segment to the previous one.
        // If the last operation of this segment is a potential data producer, join them
        // under an Apply operation.
        if (parent_op->type & OP_TAPS) {
            OpBase *op_apply = NewApplyOp();
            ExecutionPlan_PushBelow(parent_op, op_apply);
            ExecutionPlan_AddOp(op_apply, prev_op);
        } else {
            // All operations can be connected in a single chain.
            ExecutionPlan_AddOp(parent_op, prev_op);
        }
    }

    if(segment->filter_tree) {
        Vector *sub_trees = FilterTree_SubTrees(segment->filter_tree);

        /* For each filter tree find the earliest position along the execution
         * after which the filter tree can be applied. */
        for(int i = 0; i < Vector_Size(sub_trees); i++) {
            FT_FilterNode *tree;
            Vector_Get(sub_trees, i, &tree);

            rax *references = FilterTree_CollectModified(tree);

            /* Scan execution segment, locate the earliest position where all
             * references been resolved. */
            OpBase *op = ExecutionPlan_LocateReferences(segment->root, references);
            assert(op);

            /* Create filter node.
             * Introduce filter op right below located op. */
            OpBase *filter_op = NewFilterOp(tree);
            ExecutionPlan_PushBelow(op, filter_op);
            raxFree(references);
        }
        Vector_Free(sub_trees);
    }

    _associateRecordMap(segment->root, record_map);

    raxFree(resolved);

    return segment;
}

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, bool compact, bool explain) {
    AST *ast = AST_GetFromTLS();

    ExecutionPlan *plan = rm_malloc(sizeof(ExecutionPlan));

    plan->result_set = (explain) ? NULL : NewResultSet(ctx, compact);

    uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
    plan->segment_count = with_clause_count + 1;

    plan->segments = rm_malloc(plan->segment_count * sizeof(ExecutionPlanSegment));

    uint *segment_indices = NULL;
    if (with_clause_count > 0) segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);

    uint i = 0;
    uint end_offset;
    uint start_offset = 0;
    OpBase *prev_op = NULL;
    ExecutionPlanSegment *segment = NULL;
    AR_ExpNode **input_projections = NULL;

    // The original AST does not need to be modified if our query only has one segment
    AST *ast_segment = ast;
    if (with_clause_count > 0) {
        for (i = 0; i < with_clause_count; i++) {
            end_offset = segment_indices[i] + 1; // Switching from index to bound, so add 1
            ast_segment = AST_NewSegment(ast, start_offset, end_offset);
            segment =_NewExecutionPlanSegment(ctx, gc, ast_segment, plan->result_set, input_projections, prev_op);
            plan->segments[i] = segment;
            // TODO probably a memory leak on ast->root
            AST_Free(ast_segment); // Free all AST constructions scoped to this segment
            // Store the expressions constructed by this segment's WITH projection to pass into the *next* segment
            prev_op = segment->root;
            input_projections = segment->projections;
            start_offset = end_offset;
        }
        // Prepare the last AST segment
        end_offset = cypher_astnode_nchildren(ast->root);
        ast_segment = AST_NewSegment(ast, start_offset, end_offset);
    }

    segment = _NewExecutionPlanSegment(ctx, gc, ast_segment, plan->result_set, input_projections, prev_op);
    plan->segments[i] = segment;

    plan->root = plan->segments[i]->root;

    optimizePlan(gc, plan);


    if (explain == false) {
        plan->result_set->exps = segment->projections;
        ResultSet_ReplyWithPreamble(plan->result_set, segment->query_graph);
    }
    // Free current AST segment if it has been constructed here.
    if (ast_segment != ast) {
        AST_Free(ast_segment);
    }
    // _AST_Free(ast);

    return plan;
}

void _ExecutionPlan_Print(const OpBase *op, RedisModuleCtx *ctx, char *buffer, int buffer_len, int ident, int *op_count) {
    if(!op) return;

    *op_count += 1; // account for current operation.

    // Construct operation string representation.
    int bytes_written = snprintf(buffer, buffer_len, "%*s", ident, "");
    bytes_written += OpBase_ToString(op, buffer+bytes_written, buffer_len-bytes_written);

    RedisModule_ReplyWithStringBuffer(ctx, buffer, bytes_written);

    // Recurse over child operations.
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlan_Print(op->children[i], ctx, buffer, buffer_len, ident+4, op_count);
    }
}

// Reply with a string representation of given execution plan.
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx) {
    assert(plan && ctx);

    int op_count = 0;   // Number of operations printed.
    char buffer[1024];

    // No idea how many operation are in execution plan.
    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
    _ExecutionPlan_Print(plan->root, ctx, buffer, 1024, 0, &op_count);
    RedisModule_ReplySetArrayLength(ctx, op_count);
}

void _ExecutionPlanInit(OpBase *root, RecordMap *record_map) {

    // Share this ExecutionPlanSegment's record map with the operation.
    // TODO already done in segment construction
    if (root->record_map == NULL) root->record_map = record_map;

    // Initialize the operation if necesary.
    if(root->init) root->init(root);

    // Continue initializing downstream operations.
    for(int i = 0; i < root->childCount; i++) {
        _ExecutionPlanInit(root->children[i], record_map);
    }
}

void ExecutionPlanInit(ExecutionPlan *plan) {
    if(!plan) return;
    for (int i = 0; i < plan->segment_count; i ++) {
        RecordMap *segment_map = plan->segments[i]->record_map;
        _ExecutionPlanInit(plan->segments[i]->root, segment_map);
    }
}


ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    Record r;
    OpBase *op = plan->root;

    ExecutionPlanInit(plan);
    while((r = OpBase_Consume(op)) != NULL) Record_Free(r);
    return plan->result_set;
}

static void _ExecutionPlan_InitProfiling(OpBase *root) {
    root->profile = root->consume;
    root->consume = OpBase_Profile;
    root->stats = rm_malloc(sizeof(OpStats));
    root->stats->profileExecTime = 0;
    root->stats->profileRecordCount = 0;

    if(root->childCount) {
        for(int i = 0; i < root->childCount; i++) {
            OpBase *child = root->children[i];
            _ExecutionPlan_InitProfiling(child);
        }
    }
}

static void _ExecutionPlan_FinalizeProfiling(OpBase *root) {
    if(root->childCount) {
        for(int i = 0; i < root->childCount; i++) {
            OpBase *child = root->children[i];
            root->stats->profileExecTime -= child->stats->profileExecTime;
            _ExecutionPlan_FinalizeProfiling(child);
        }
    }
    root->stats->profileExecTime *= 1000;   // Milliseconds.
}

ResultSet* ExecutionPlan_Profile(ExecutionPlan *plan) {
    _ExecutionPlan_InitProfiling(plan->root);
    ResultSet *rs = ExecutionPlan_Execute(plan);
    _ExecutionPlan_FinalizeProfiling(plan->root);
    return rs;
}

void _ExecutionPlan_FreeOperations(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlan_FreeOperations(op->children[i]);
    }
    OpBase_Free(op);
}

void _ExecutionPlanSegment_Free(ExecutionPlanSegment *segment) {
    RecordMap_Free(segment->record_map);

    if (segment->connected_components) {
        uint connected_component_count = array_len(segment->connected_components);
        for (uint i = 0; i < connected_component_count; i ++) {
            QueryGraph_Free(segment->connected_components[i]);
        }
        array_free(segment->connected_components);
    }

    QueryGraph_Free(segment->query_graph);

    if (segment->projections) {
        uint projection_count = array_len(segment->projections);
        for (uint i = 0; i < projection_count; i ++) {
            AR_EXP_Free(segment->projections[i]);
        }
        array_free(segment->projections);
    }

    if (segment->order_expressions) {
        uint order_count = array_len(segment->order_expressions);
        for (uint i = 0; i < order_count; i ++) {
            AR_EXP_Free(segment->order_expressions[i]);
        }
        array_free(segment->order_expressions);
    }

    // TODO FT?
    rm_free(segment);
}

void ExecutionPlan_Free(ExecutionPlan *plan) {
    if(plan == NULL) return;
    _ExecutionPlan_FreeOperations(plan->root);

    for (uint i = 0; i < plan->segment_count; i ++) {
        _ExecutionPlanSegment_Free(plan->segments[i]);
    }
    rm_free(plan->segments);

    rm_free(plan);
}

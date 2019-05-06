/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/arr.h"
#include "../util/vector.h"
#include "../query_executor.h"
#include "../graph/entities/edge.h"
#include "./optimizations/optimizer.h"
#include "./optimizations/optimizations.h"
#include "../arithmetic/algebraic_expression.h"
#include "../parser/ast_build_op_contexts.h"
#include "../parser/ast_build_filter_tree.h"

/* Given an AST path, construct a series of scans and traversals to model it. */
void _ExecutionPlanSegment_BuildTraversalOps(QueryGraph *qg, FT_FilterNode *ft, const cypher_astnode_t *path, Vector *traversals) {
    GraphContext *gc = GraphContext_GetFromTLS();
    AST *ast = AST_GetFromTLS();
    OpBase *op = NULL;

    uint nelems = cypher_ast_pattern_path_nelements(path);
    if (nelems == 1) {
        // Only one entity is specified - build a node scan.
        const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, 0);
        AR_ExpNode *ar_exp = AST_GetEntity(ast, ast_node);
        // Register entity for Record if necessary
        AST_RecordAccommodateExpression(ast, ar_exp);

        uint rec_idx = ar_exp->record_idx;
        Node *n = QueryGraph_GetEntityByASTRef(qg, ast_node);
        if(cypher_ast_node_pattern_nlabels(ast_node) > 0) {
            op = NewNodeByLabelScanOp(gc, n, rec_idx);
        } else {
            op = NewAllNodeScanOp(gc->g, n, rec_idx);
        }
        Vector_Push(traversals, op);
        return;
    }

    // This path must be expressed with one or more traversals.
    size_t expCount = 0;
    AlgebraicExpression **exps = AlgebraicExpression_FromPath(ast, qg, path, &expCount);

    TRAVERSE_ORDER order;
    if (exps[0]->op == AL_EXP_UNARY) {
        // If either the first or last expression simply specifies a node, it should
        // be replaced by a label scan. (This can be the case after building a
        // variable-length traversal like MATCH (a)-[*]->(b:labeled)
        AlgebraicExpression *to_replace = exps[0];
        if (to_replace->src_node_idx == NOT_IN_RECORD) {
            // Anonymous node - make space for it in the Record
            to_replace->src_node_idx = AST_AddAnonymousRecordEntry(ast);
        }
        op = NewNodeByLabelScanOp(gc, to_replace->src_node, to_replace->src_node_idx);
        Vector_Push(traversals, op);
        AlgebraicExpression_Free(to_replace);
        for (uint q = 1; q < expCount; q ++) {
            exps[q-1] = exps[q];
        }
        expCount --;
        order = TRAVERSE_ORDER_FIRST;
    } else if (exps[expCount - 1]->op == AL_EXP_UNARY) {
        AlgebraicExpression *to_replace = exps[expCount - 1];
        if (to_replace->src_node_idx == NOT_IN_RECORD) {
            // Anonymous node - make space for it in the Record
            to_replace->src_node_idx = AST_AddAnonymousRecordEntry(ast);
        }
        op = NewNodeByLabelScanOp(gc, to_replace->src_node, to_replace->src_node_idx);
        Vector_Push(traversals, op);
        AlgebraicExpression_Free(to_replace);
        expCount --;
        order = TRAVERSE_ORDER_LAST;
    } else {
        order = determineTraverseOrder(ft, exps, expCount);
    }

    if(order == TRAVERSE_ORDER_FIRST) {
        if (op == NULL) {
            // We haven't already built the appropriate label scan
            AlgebraicExpression *exp = exps[0];
            selectEntryPoint(exp, ft);
            if (exp->src_node_idx == NOT_IN_RECORD) {
                // Anonymous node - make space for it in the Record
                exp->src_node_idx = AST_AddAnonymousRecordEntry(ast);
            }

            // Create SCAN operation.
            if(exp->src_node->label) {
                /* There's no longer need for the last matrix operand
                 * as it's been replaced by label scan. */
                AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, NULL);
                op = NewNodeByLabelScanOp(gc, exp->src_node, exp->src_node_idx);
                Vector_Push(traversals, op);
            } else {
                op = NewAllNodeScanOp(gc->g, exp->src_node, exp->src_node_idx);
                Vector_Push(traversals, op);
            }
        }
        for(int i = 0; i < expCount; i++) {
            if(exps[i]->operand_count == 0) continue;
            // TODO tmp
            if (exps[i]->op == AL_EXP_UNARY) {
                 exps[i]->dest_node_idx = exps[i]->src_node_idx;
            } else {
                AlgebraicExpression_ExtendRecord(exps[i]);
            }
            if(exps[i]->minHops != 1 || exps[i]->maxHops != 1) {
                op = NewCondVarLenTraverseOp(exps[i],
                                             exps[i]->minHops,
                                             exps[i]->maxHops,
                                             gc->g);
            } else {
                op = NewCondTraverseOp(gc->g, exps[i], TraverseRecordCap(ast));
            }
            Vector_Push(traversals, op);
        }
    } else {
        if (op == NULL) {
            // We haven't already built the appropriate label scan
            AlgebraicExpression *exp = exps[expCount-1];
            selectEntryPoint(exp, ft);

            if (exp->dest_node_idx == NOT_IN_RECORD) {
                // Anonymous node - make space for it in the Record
                exp->dest_node_idx = AST_AddAnonymousRecordEntry(ast);
            }

            // Create SCAN operation.
            if(exp->dest_node->label) {
                /* There's no longer need for the last matrix operand
                 * as it's been replaced by label scan. */
                AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, NULL);
                op = NewNodeByLabelScanOp(gc, exp->dest_node, exp->dest_node_idx);
                Vector_Push(traversals, op);
            } else {
                op = NewAllNodeScanOp(gc->g, exp->dest_node, exp->dest_node_idx);
                Vector_Push(traversals, op);
            }
        }

        for(int i = expCount-1; i >= 0; i--) {
            if(exps[i]->operand_count == 0) continue;
            AlgebraicExpression_Transpose(exps[i]);
            // TODO tmp
            if (exps[i]->op == AL_EXP_UNARY) {
                exps[i]->src_node_idx = exps[i]->dest_node_idx;
            } else {
                AlgebraicExpression_ExtendRecord(exps[i]);
            }
            if(exps[i]->minHops != 1 || exps[i]->maxHops != 1) {
                op = NewCondVarLenTraverseOp(exps[i],
                                             exps[i]->minHops,
                                             exps[i]->maxHops,
                                             gc->g);
            } else {
                op = NewCondTraverseOp(gc->g, exps[i], TraverseRecordCap(ast));
            }
            Vector_Push(traversals, op);
        }
    }
    // Free the expressions array, as its parts have been converted into operations
    free(exps);
}

void _ExecutionPlanSegment_AddTraversalOps(Vector *ops, OpBase *cartesian_root, Vector *traversals) {
    if(cartesian_root) {
        // If we're traversing multiple disjoint paths, the new traversal
        // should be connected uner a Cartesian product.
        OpBase *childOp;
        OpBase *parentOp;
        Vector_Pop(traversals, &parentOp);
        // Connect cartesian product to the root of traversal.
        ExecutionPlanSegment_AddOp(cartesian_root, parentOp);
        while(Vector_Pop(traversals, &childOp)) {
            ExecutionPlanSegment_AddOp(parentOp, childOp);
            parentOp = childOp;
        }
    } else {
        // Otherwise, the traversals can be added sequentially to the overall ops chain
        OpBase *op;
        for(int traversalIdx = 0; traversalIdx < Vector_Size(traversals); traversalIdx++) {
            Vector_Get(traversals, traversalIdx, &op);
            Vector_Push(ops, op);
        }
    }
}

ExecutionPlanSegment* _NewExecutionPlanSegment(RedisModuleCtx *ctx, GraphContext *gc, AST *ast, ResultSet *result_set) {
    ExecutionPlanSegment *segment = malloc(sizeof(ExecutionPlanSegment));
    Vector *ops = NewVector(OpBase*, 1);

    // Build query graph
    QueryGraph *qg = BuildQueryGraph(gc, ast);
    segment->query_graph = qg;

    // Build filter tree
    FT_FilterNode *filter_tree = AST_BuildFilterTree(ast);
    segment->filter_tree = filter_tree;

    const cypher_astnode_t **match_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_MATCH);
    uint match_count = array_len(match_clauses);

    /* TODO Currently, we don't differentiate between:
     * MATCH (a) MATCH (b)
     * and
     * MATCH (a), (b)
     * Introduce this distinction. */
    OpBase *cartesianProduct = NULL;
    if (match_count > 1) {
        cartesianProduct = NewCartesianProductOp(AST_RecordLength(ast));
        Vector_Push(ops, cartesianProduct);
    }

    // Build traversal operations for every MATCH clause
    for (uint i = 0; i < match_count; i ++) {
        // Each MATCH clause has a pattern that consists of 1 or more paths
        const cypher_astnode_t *ast_pattern = cypher_ast_match_get_pattern(match_clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(ast_pattern);

        /* If we're dealing with multiple paths (which our validations have guaranteed
         * are disjoint), we'll join them all together with a Cartesian product (full join). */
        if ((cartesianProduct == NULL) && (cypher_ast_pattern_npaths(ast_pattern) > 1)) {
            cartesianProduct = NewCartesianProductOp(AST_RecordLength(ast));
            Vector_Push(ops, cartesianProduct);
        }

        Vector *path_traversal = NewVector(OpBase*, 1);
        for (uint j = 0; j < npaths; j ++) {
            // Convert each path into the appropriate traversal operation(s).
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(ast_pattern, j);
            _ExecutionPlanSegment_BuildTraversalOps(qg, filter_tree, path, path_traversal);
            _ExecutionPlanSegment_AddTraversalOps(ops, cartesianProduct, path_traversal);
            Vector_Clear(path_traversal);
        }
        Vector_Free(path_traversal);
    }

    array_free(match_clauses);

    // Set root operation
    const cypher_astnode_t *unwind_clause = AST_GetClause(ast, CYPHER_AST_UNWIND);
    if(unwind_clause) {
        AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(ast, unwind_clause);

        OpBase *opUnwind = NewUnwindOp(unwind_ast_ctx.record_len, unwind_ast_ctx.record_idx, unwind_ast_ctx.exps, unwind_ast_ctx.alias);
        Vector_Push(ops, opUnwind);
    }

    bool create_clause = AST_ContainsClause(ast, CYPHER_AST_CREATE);
    if(create_clause) {
        AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(ast, qg);
        OpBase *opCreate = NewCreateOp(&result_set->stats,
                                       create_ast_ctx.nodes_to_create,
                                       create_ast_ctx.edges_to_create,
                                       create_ast_ctx.record_len);
        Vector_Push(ops, opCreate);
    }

    const cypher_astnode_t *merge_clause = AST_GetClause(ast, CYPHER_AST_MERGE);
    if(merge_clause) {
        // A merge clause provides a single path that must exist or be created.
        // As with paths in a MATCH query, build the appropriate traversal operations
        // and append them to the set of ops.
        const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);
        Vector *path_traversal = NewVector(OpBase*, 1);
        _ExecutionPlanSegment_BuildTraversalOps(qg, filter_tree, path, path_traversal);
        _ExecutionPlanSegment_AddTraversalOps(ops, NULL, path_traversal);
        Vector_Free(path_traversal);

        // Append a merge operation
        AST_MergeContext merge_ast_ctx = AST_PrepareMergeOp(ast, merge_clause, qg);
        OpBase *opMerge = NewMergeOp(&result_set->stats,
                                     merge_ast_ctx.nodes_to_merge,
                                     merge_ast_ctx.edges_to_merge,
                                     merge_ast_ctx.record_len);
        Vector_Push(ops, opMerge);
    }

    const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
    if(delete_clause) {
        uint *nodes_ref;
        uint *edges_ref;
        AST_PrepareDeleteOp(delete_clause, &nodes_ref, &edges_ref);
        OpBase *opDelete = NewDeleteOp(nodes_ref, edges_ref, &result_set->stats);
        Vector_Push(ops, opDelete);
    }

    const cypher_astnode_t *set_clause = AST_GetClause(ast, CYPHER_AST_SET);
    if(set_clause) {
        // Create a context for each update expression.
        uint nitems;
        EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(set_clause, &nitems);
        OpBase *op_update = NewUpdateOp(gc, update_exps, nitems, &result_set->stats);
        Vector_Push(ops, op_update);
    }

    AR_ExpNode **exps = NULL;
    uint *modifies = NULL;
    bool aggregate = false;

    const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
    if(with_clause) {
        assert(false);
    }

    const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
    if(ret_clause) {
        uint exp_count = array_len(ast->return_expressions);
        exps = array_new(AR_ExpNode*, exp_count);
        modifies = array_new(uint, exp_count);
        for (uint i = 0; i < exp_count; i ++) {
            // TODO maybe store these exps in AST
            AR_ExpNode *exp = AST_GetEntityFromAlias(ast, (char*)ast->return_expressions[i]);
            exps = array_append(exps, exp);
            if (exp->record_idx) modifies = array_append(modifies, exp->record_idx);
        }
        // TODO We've already determined this during AST validations, could refactor to make
        // this call unnecessary.
        aggregate = AST_ClauseContainsAggregation(ret_clause);
    }


    OpBase *op;
    if(ret_clause || with_clause) {
        if(aggregate) op = NewAggregateOp(exps, modifies);
        else op = NewProjectOp(exps, modifies);
        Vector_Push(ops, op);
    }


    if (ret_clause) {
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
        if (limit_clause) limit = skip + AST_ParseIntegerNode(limit_clause);

        if (order_clause) {
            int direction;
            AR_ExpNode **expressions = AST_PrepareSortOp(order_clause, &direction);
            op = NewSortOp(expressions, direction, limit);
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
    }

    OpBase *parent_op;
    OpBase *child_op;
    Vector_Pop(ops, &parent_op);
    segment->root = parent_op;

    while(Vector_Pop(ops, &child_op)) {
        ExecutionPlanSegment_AddOp(parent_op, child_op);
        parent_op = child_op;
    }

    Vector_Free(ops);

    if(segment->filter_tree) {
        Vector *sub_trees = FilterTree_SubTrees(segment->filter_tree);

        /* For each filter tree find the earliest position along the execution
         * after which the filter tree can be applied. */
        for(int i = 0; i < Vector_Size(sub_trees); i++) {
            FT_FilterNode *tree;
            Vector_Get(sub_trees, i, &tree);

            uint *references = FilterTree_CollectModified(tree);

            /* Scan execution segment, locate the earliest position where all
             * references been resolved. */
            OpBase *op = ExecutionPlanSegment_LocateReferences(segment->root, references);
            assert(op);

            /* Create filter node.
             * Introduce filter op right below located op. */
            OpBase *filter_op = NewFilterOp(tree);
            ExecutionPlanSegment_PushBelow(op, filter_op);
            array_free(references);
        }
        Vector_Free(sub_trees);
    }

    optimizeSegment(gc, segment);
    return segment;
}

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, bool explain) {
    AST *ast = AST_GetFromTLS();

    ExecutionPlan *plan = malloc(sizeof(ExecutionPlan));

    plan->result_set = NULL;
    if(!explain) {
        plan->result_set = NewResultSet(ctx);
        ResultSet_CreateHeader(plan->result_set, ast->return_expressions);
    }

    uint segment_count = AST_GetClauseCount(ast, CYPHER_AST_WITH) + 1;
    plan->segment_count = segment_count;

    uint *segment_indices = NULL;
    if (segment_count > 1) segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);

    plan->segments = malloc(segment_count * sizeof(ExecutionPlanSegment));
    uint i;
    for (i = 0; i < segment_count - 1; i ++) {
        ast->end_offset = segment_indices[i] + 1; // Switching from index to bound, so add 1
        plan->segments[i] = _NewExecutionPlanSegment(ctx, gc, ast, plan->result_set);
        ast->start_offset = ast->end_offset;
    }

    ast->end_offset = AST_NumClauses(ast);
    plan->segments[i] = _NewExecutionPlanSegment(ctx, gc, ast, plan->result_set);

    return plan;
}

void _ExecutionPlanSegmentPrint(const OpBase *op, char **strPlan, int ident) {
    char strOp[512] = {0};
    sprintf(strOp, "%*s%s\n", ident, "", op->name);

    if(*strPlan == NULL) {
        *strPlan = calloc(strlen(strOp) + 1, sizeof(char));
    } else {
        *strPlan = realloc(*strPlan, sizeof(char) * (strlen(*strPlan) + strlen(strOp) + 2));
    }
    strcat(*strPlan, strOp);

    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanSegmentPrint(op->children[i], strPlan, ident + 4);
    }
}

char* ExecutionPlan_Print(const ExecutionPlan *plan) {
    char *strPlan = NULL;
    for (uint i = 0; i < plan->segment_count; i ++) {
        _ExecutionPlanSegmentPrint(plan->segments[i]->root, &strPlan, 0);
    }
    return strPlan;
}

void _ExecutionPlanSegmentInit(OpBase *root) {
    if(root->init) root->init(root);
    for(int i = 0; i < root->childCount; i++) {
        _ExecutionPlanSegmentInit(root->children[i]);
    }
}

void ExecutionPlanSegmentInit(ExecutionPlanSegment *plan) {
    if(!plan) return;
    _ExecutionPlanSegmentInit(plan->root);
}

Record ExecutionPlanSegment_Execute(ExecutionPlanSegment *plan) {
    OpBase *op = plan->root;

    ExecutionPlanSegmentInit(plan);
    Record r = op->consume(op);

    return r;
}

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    Record r = Record_New(0); // TODO tmp
    while (r) {
        for (int i = 0; i < plan->segment_count; i ++) {
            r = ExecutionPlanSegment_Execute(plan->segments[i]);
        }
    }
    return plan->result_set;
}

void _ExecutionPlanSegmentFreeRecursive(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanSegmentFreeRecursive(op->children[i]);
    }
    OpBase_Free(op);
}

void _ExecutionPlanSegmentFree(ExecutionPlanSegment *plan) {
    if(plan == NULL) return;
    if(plan->root) _ExecutionPlanSegmentFreeRecursive(plan->root);

    QueryGraph_Free(plan->query_graph);
    free(plan);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    if(plan == NULL) return;
    for (uint i = 0; i < plan->segment_count; i ++) {
        _ExecutionPlanSegmentFree(plan->segments[i]);
    }

    free(plan);
}

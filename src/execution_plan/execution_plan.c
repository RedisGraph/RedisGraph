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

/* Checks if parent has given child, if so returns 1
 * otherwise returns 0 */
int _OpBase_ContainsChild(const OpBase *parent, const OpBase *child) {
    for(int i = 0; i < parent->childCount; i++) {
        if(parent->children[i] == child) {
            return 1;
        }
    }
    return 0;
}

void _OpBase_AddChild(OpBase *parent, OpBase *child) {
    // Add child to parent
    if(parent->children == NULL) {
        parent->children = rm_malloc(sizeof(OpBase *));
    } else {
        parent->children = rm_realloc(parent->children, sizeof(OpBase *) * (parent->childCount+1));
    }
    parent->children[parent->childCount++] = child;

    // Add parent to child
    child->parent = parent;
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpBase_RemoveNode(OpBase *parent, OpBase *child) {
    // Remove child from parent.
    int i = 0;
    for(; i < parent->childCount; i++) {
        if(parent->children[i] == child) break;
    }
    
    assert(i != parent->childCount);    

    // Uppdate child count.
    parent->childCount--;
    if(parent->childCount == 0) {
        rm_free(parent->children);
        parent->children = NULL;
    } else {
        // Shift left children.
        for(int j = i; j < parent->childCount; j++) {
            parent->children[j] = parent->children[j+1];
        }
        parent->children = rm_realloc(parent->children, sizeof(OpBase *) * parent->childCount);
    }

    // Remove parent from child.
    child->parent = NULL;
}

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
    _OpBase_RemoveNode(parent, child);
}

Vector* _ExecutionPlan_Locate_References(OpBase *root, OpBase **op, rax *references) {
    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * previously modified entities (up the execution plan). */
    Vector *seen = NewVector(char*, 0);
    char *modifiedEntity;

    /* Append current op modified entities. */
    if(root->modifies) {
        for(int i = 0; i < Vector_Size(root->modifies); i++) {
            Vector_Get(root->modifies, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
        }
    }

     /* Traverse execution plan, upwards. */
    for(int i = 0; i < root->childCount; i++) {
        Vector *saw = _ExecutionPlan_Locate_References(root->children[i], op, references);

        /* Quick return if op was located. */
        if(*op) {
            Vector_Free(saw);
            return seen;
        }

        /* Add modified entities from previous operation. */
        for(int i = 0; i < Vector_Size(saw); i++) {
            Vector_Get(saw, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
        }
        Vector_Free(saw);
    }

    /* See if all references been resolved.
     * TODO: create a vector compare function
     * which checks if the content of one is in the other. */
    int match = raxSize(references);
    int ref_count = match;
    int seen_count = Vector_Size(seen);
    
    // We've seen enough to start matching.
    if(seen_count >= ref_count) {
        for(int i = 0; i < seen_count; i++) {
            // To many unmatched references.
            if(match > (seen_count-i)) break;

            char *resolved;
            Vector_Get(seen, i, &resolved);
            if(raxFind(references, (unsigned char*)resolved, strlen(resolved)) != raxNotFound) {
                match--;
                // All refrences been resolved.
                if(match == 0) {
                    *op = root;
                    break;
                }
            }
        }
    }

    return seen;
}

void _Count_Graph_Entities(const Vector *entities, size_t *node_count, size_t *edge_count) {
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);

        if(entity->t == N_ENTITY) {
            (*node_count)++;
        } else if(entity->t == N_LINK) {
            (*edge_count)++;
        }
    }
}

void _Determine_Graph_Size(const AST *ast, size_t *node_count, size_t *edge_count) {
    *edge_count = 0;
    *node_count = 0;
    Vector *entities;
    
    if(ast->matchNode) {
        entities = ast->matchNode->_mergedPatterns;
        _Count_Graph_Entities(entities, node_count, edge_count);
    }

    if(ast->createNode) {
        entities = ast->createNode->graphEntities;
        _Count_Graph_Entities(entities, node_count, edge_count);
    }
}

void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
    _OpBase_AddChild(parent, newOp);
}

void ExecutionPlan_PushBelow(OpBase *a, OpBase *b) {
    /* B is a new operation. */
    assert(!(b->parent || b->children));
    assert(a->parent);

    /* Replace A's former parent. */
    OpBase *a_former_parent = a->parent;

    /* Disconnect A from its former parent. */
    _OpBase_RemoveChild(a_former_parent, a);

    /* Add A's former parent as parent of B. */
    _OpBase_AddChild(a_former_parent, b);

    /* Add A as a child of B. */
    _OpBase_AddChild(b, a);
}

void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b) {
    // Insert the new operation between the original and its parent.
    ExecutionPlan_PushBelow(a, b);
    // Delete the original operation.
    ExecutionPlan_RemoveOp(plan, a);
}

void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op) {
    if(op->parent == NULL) {
        // Removing execution plan root.
        assert(op->childCount == 1);
        plan->root = op->children[0];
    } else {
        // Remove op from its parent.
        OpBase* parent = op->parent;
        _OpBase_RemoveChild(op->parent, op);

        // Add each of op's children as a child of op's parent.
        for(int i = 0; i < op->childCount; i++) {
            _OpBase_AddChild(parent, op->children[i]);
        }
    }

    // Clear op.
    op->parent = NULL;
    rm_free(op->children);
    op->children = NULL;
    op->childCount = 0;
}

static bool inline _TapOperation(const OpBase *op) {
    return (op->type == OPType_ALL_NODE_SCAN ||
       op->type == OPType_NODE_BY_LABEL_SCAN ||
       op->type == OPType_INDEX_SCAN ||
       op->type == OPType_CREATE ||
       op->type == OPType_UNWIND ||
       op->type == OPType_PROC_CALL);
}

void ExecutionPlan_LocateTaps(OpBase *root, OpBase ***taps) {
    if(root == NULL) return;
    if(_TapOperation(root)) *taps = array_append(*taps, root);
    for(int i = 0; i < root->childCount; i++) {
        ExecutionPlan_LocateTaps(root->children[i], taps);
    }
}

OpBase* ExecutionPlan_LocateOp(OpBase *root, OPType type) {
    if(!root) return NULL;

    if(root->type == type) {
        return root;
    }

    for(int i = 0; i < root->childCount; i++) {
        OpBase *op = ExecutionPlan_LocateOp(root->children[i], type);
        if(op) return op;
    }

    return NULL;
}

void _ExecutionPlan_LocateOps(OpBase *root, OPType type, OpBase ***ops) {
    if(!root) return;

    if(root->type & type) (*ops) = array_append((*ops), root);

    for(int i = 0; i < root->childCount; i++) {
        _ExecutionPlan_LocateOps(root->children[i], type, ops);
    }
}

OpBase** ExecutionPlan_LocateOps(OpBase *root, OPType type) {
    OpBase **ops = array_new(OpBase*, 0);
    _ExecutionPlan_LocateOps(root, type, &ops);
    return ops;
}

void ExecutionPlan_Taps(OpBase *root, OpBase ***taps) {
    if(root == NULL) return;
    if(root->type & OP_SCAN) *taps = array_append(*taps, root);

    for(int i = 0; i < root->childCount; i++) {
        ExecutionPlan_Taps(root->children[i], taps);
    }
}

OpBase* ExecutionPlan_Locate_References(OpBase *root, rax *references) {
    OpBase *op = NULL;    
    Vector *temp = _ExecutionPlan_Locate_References(root, &op, references);
    Vector_Free(temp);
    return op;
}

// TODO: get rid of _ReturnClause_GetExpressions, _WithClause_GetExpressions and _OrderClause_GetExpressions.
/* Returns an array of arithmetic expression, one for every return element.
 * caller is responsible for freeing each arithmetic expression in addition
 * to the array itself. */
static AR_ExpNode** _ReturnClause_GetExpressions(const AST *ast) {
    assert(ast->returnNode);

    AST_ReturnNode *return_node = ast->returnNode;
    unsigned int elem_count = array_len(return_node->returnElements);
    AR_ExpNode **exps = array_new(AR_ExpNode*, elem_count);

    for(unsigned int i = 0; i < elem_count; i++) {
        AST_ReturnElementNode *elem = return_node->returnElements[i];
        AR_ExpNode *exp = AR_EXP_BuildFromAST(ast, elem->exp);
        exps = array_append(exps, exp);
    }

    return exps;
}

/* Returns an array of arithmetic expression, one for every wiht element.
 * caller is responsible for freeing each arithmetic expression in addition
 * to the array itself. */
static AR_ExpNode** _WithClause_GetExpressions(const AST *ast) {
    assert(ast->withNode);

    AST_WithNode *with_node = ast->withNode;
    unsigned int elem_count = array_len(with_node->exps);
    AR_ExpNode **exps = array_new(AR_ExpNode*, elem_count);

    for(unsigned int i = 0; i < elem_count; i++) {
        AST_WithElementNode *elem = with_node->exps[i];
        AR_ExpNode *exp = AR_EXP_BuildFromAST(ast, elem->exp);
        exps = array_append(exps, exp);
    }

    return exps;
}

/* Returns an array of arithmetic expression, one for every order element.
 * caller is responsible for freeing each arithmetic expression in addition
 * to the array itself. */
static AR_ExpNode** _OrderClause_GetExpressions(const AST *ast) {
    assert(ast && ast->orderNode);
	AST_OrderNode *order_node = ast->orderNode;

	unsigned int exp_count = array_len(order_node->expressions);
	AR_ExpNode** exps = array_new(AR_ExpNode*, exp_count);

	for(unsigned int i = 0; i < exp_count; i++) {
		AR_ExpNode *exp = AR_EXP_BuildFromAST(ast, order_node->expressions[i]);
		exps = array_append(exps, exp);
	}

	return exps;
}

/* Keep track after resolved variabels.
 * add variabels modified/set by op to resolved. */ 
static void _UpdateResolvedVariables(rax *resolved, OpBase *op) {
    assert(resolved && op);
    if(!op->modifies) return;

    int count = Vector_Size(op->modifies);
    for(int i = 0; i < count; i++) {
        char *var;
        Vector_Get(op->modifies, i, &var);
        raxInsert(resolved, (unsigned char*)var, strlen(var), NULL, NULL);
    }
}

ExecutionPlan* _NewExecutionPlan(RedisModuleCtx *ctx, AST *ast, ResultSet *result_set) {
    Graph *g;
    OpBase *op;
    Vector *ops;
    rax *resolved;
    QueryGraph *q;
    GraphContext *gc;
    size_t node_count;
    size_t edge_count;
    FT_FilterNode *filter_tree;
    ExecutionPlan *execution_plan;

    resolved = raxNew();
    ops = NewVector(OpBase*, 1);
    gc = GraphContext_GetFromTLS();
    g = gc->g;
    execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));
    execution_plan->result_set = result_set;

    /* Predetermin graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    _Determine_Graph_Size(ast, &node_count, &edge_count);
    q = QueryGraph_New(node_count, edge_count);

    filter_tree = NULL;
    if(ast->whereNode) {
        filter_tree = BuildFiltersTree(ast, ast->whereNode->filters);
        execution_plan->filter_tree = filter_tree;
    }

    if(ast->callNode) {        
        OpBase *opProcCall = NewProcCallOp(ast->callNode->procedure,
                                            ast->callNode->arguments,
                                            ast->callNode->yield, ast);
        Vector_Push(ops, opProcCall);
        _UpdateResolvedVariables(resolved, opProcCall);
    }

    if(ast->matchNode) {
        BuildQueryGraph(gc, q, ast->matchNode->_mergedPatterns);

        QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(q);
        size_t connectedComponentsCount = array_len(connectedComponents);
        execution_plan->connected_components = connectedComponents;

        /* For every connected component.
         * Incase we're dealing with multiple components
         * we'll simply join them all together with a join operation. */
        OpBase *cartesianProduct = NULL;
        if(connectedComponentsCount > 1) {
            cartesianProduct = NewCartesianProductOp(AST_AliasCount(ast));
            Vector_Push(ops, cartesianProduct);
        }
        
        // Keep track after all traversal operations along a pattern.
        Vector *traversals = NewVector(OpBase*, 1);

        for(int i = 0; i < connectedComponentsCount; i++) {
            QueryGraph *cc = connectedComponents[i];
            if(cc->edge_count == 0) {
                /* Node scan. */
                Node *n = cc->nodes[0];
                if(n->label) op = NewNodeByLabelScanOp(n, ast);
                else op = NewAllNodeScanOp(g, n, ast);
                Vector_Push(traversals, op);
                _UpdateResolvedVariables(resolved, op);
            } else {
                size_t expCount = 0;
                AlgebraicExpression **exps = AlgebraicExpression_From_QueryGraph(cc, ast, &expCount);

                // Reorder exps, to the most performent arrangement of evaluation.
                orderExpressions(exps, expCount, filter_tree);

                AlgebraicExpression *exp = exps[0];
                selectEntryPoint(exp, filter_tree);

                // Create SCAN operation.
                if(exp->src_node->label) {
                    /* There's no longer need for the last matrix operand
                     * as it's been replaced by label scan. */
                    AlgebraicExpression_RemoveTerm(exp, 0, NULL);
                    op = NewNodeByLabelScanOp(exp->src_node, ast);
                } else {
                    op = NewAllNodeScanOp(g, exp->src_node, ast);
                }

                Vector_Push(traversals, op);
                _UpdateResolvedVariables(resolved, op);

                for(int i = 0; i < expCount; i++) {
                    exp = exps[i];
                    if(exp->operand_count == 0) continue;
                    
                    /* Make sure expression source is already resolved.
                     * TODO: it might be better to ensure beforehand 
                     * that exp's source is already resolved, see 
                     * traverse_order.c */
                    if(raxFind(resolved,
                        (unsigned char*)exp->src_node->alias,
                        strlen(exp->src_node->alias)) == raxNotFound) {

                        AlgebraicExpression_Transpose(exp);

                        assert(raxFind(resolved,
                            (unsigned char*)exp->src_node->alias,
                            strlen(exp->src_node->alias)) != raxNotFound);
                    }

                    if(exp->edge && Edge_VariableLength(exp->edge)) {
                        op = NewCondVarLenTraverseOp(exp,
                                                        exp->edge->minHops,
                                                        exp->edge->maxHops,
                                                        g,
                                                        ast);
                    }
                    else {
                        op = NewCondTraverseOp(exp, ast);
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
                _OpBase_AddChild(cartesianProduct, parentOp);
                while(Vector_Pop(traversals, &childOp)) {
                    _OpBase_AddChild(parentOp, childOp);
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

    if(ast->unwindNode) {
        OpBase *opUnwind = NewUnwindOp(ast);
        Vector_Push(ops, opUnwind);
        _UpdateResolvedVariables(resolved, opUnwind);
    }

    /* Set root operation */
    if(ast->createNode) {
        BuildQueryGraph(gc, q, ast->createNode->graphEntities);
        OpBase *opCreate = NewCreateOp(ctx, ast, q, execution_plan->result_set);

        Vector_Push(ops, opCreate);
        _UpdateResolvedVariables(resolved, opCreate);
    }

    if(ast->mergeNode) {
        OpBase *opMerge = NewMergeOp(ast, execution_plan->result_set);
        Vector_Push(ops, opMerge);
        _UpdateResolvedVariables(resolved, opMerge);
    }

    if(ast->deleteNode) {
        OpBase *opDelete = NewDeleteOp(ast->deleteNode, q, execution_plan->result_set, ast);
        Vector_Push(ops, opDelete);
    }

    if(ast->setNode) {
        OpBase *op_update = NewUpdateOp(ast, execution_plan->result_set);
        Vector_Push(ops, op_update);
    }

    char **aliases = NULL;  // Array of aliases RETURN n.v as V
    AR_ExpNode **exps = NULL;
    bool aggregate = false;

    if(ast->withNode) {
        exps = _WithClause_GetExpressions(ast);
        aliases = WithClause_GetAliases(ast->withNode);
        aggregate = WithClause_ContainsAggregation(ast->withNode);
    }

    if(ast->returnNode) {
        exps = _ReturnClause_GetExpressions(ast);
        aliases = ReturnClause_GetAliases(ast->returnNode);
        aggregate = ReturnClause_ContainsAggregation(ast->returnNode);
    }

    if(ast->returnNode || ast->withNode) {
        if(aggregate) op = NewAggregateOp(ast, exps, aliases);
        else op = NewProjectOp(ast, exps, aliases);
        Vector_Push(ops, op);
    }

    if(ast->returnNode && ast->returnNode->distinct) {
        op = NewDistinctOp();
        Vector_Push(ops, op);
    }

    if(ast->orderNode) {
        op = NewSortOp(ast, _OrderClause_GetExpressions(ast));
        Vector_Push(ops, op);
    }

    if(ast->skipNode) {
        OpBase *op_skip = NewSkipOp(ast->skipNode->skip);
        Vector_Push(ops, op_skip);
    }

    if(ast->limitNode) {
        OpBase *op_limit = NewLimitOp(ast->limitNode->limit);
        Vector_Push(ops, op_limit);
    }

    if(ast->returnNode) {
        op = NewResultsOp(execution_plan->result_set, q);
        Vector_Push(ops, op);
    }

    OpBase *parent_op;
    OpBase *child_op;
    Vector_Pop(ops, &parent_op);
    execution_plan->root = parent_op;

    while(Vector_Pop(ops, &child_op)) {
        _OpBase_AddChild(parent_op, child_op);
        parent_op = child_op;
    }

    Vector_Free(ops);
    raxFree(resolved);
    return execution_plan;
}

// Locates all "taps" (entry points) of root.
static void _ExecutionPlan_StreamTaps(OpBase *root, OpBase ***taps) {    
    if(root->childCount) {
        for(int i = 0; i < root->childCount; i++) {
            OpBase *child = root->children[i];
            _ExecutionPlan_StreamTaps(child, taps);
        }
    } else {
        *taps = array_append(*taps, root);
    }
}

static ExecutionPlan *_ExecutionPlan_Connect(ExecutionPlan *a, ExecutionPlan *b, AST *ast) {
    assert(a &&
           b &&
           (a->root->type == OPType_PROJECT || a->root->type == OPType_AGGREGATE));

    OpBase *tap;
    OpBase **taps = array_new(sizeof(OpBase*), 1);
    _ExecutionPlan_StreamTaps(b->root, &taps);

    unsigned short tap_count = array_len(taps);
    if(tap_count == 1 && !(taps[0]->type & OP_SCAN)) {
        /* Single tap, entry point isn't a SCAN operation, e.g.
         * MATCH (b) WITH b.v AS V RETURN V
         * MATCH (b) WITH b.v+1 AS V CREATE (n {v:V}) */
        _OpBase_AddChild(taps[0], a->root);
    } else {
        /* Multiple taps or a single SCAN tap, e.g. 
         * MATCH (b) WITH b.v AS V MATCH (c) return V,c
         * MATCH (b) WITH b.v AS V MATCH (c),(d) return c, V, d */
        for(int i = 0; i < tap_count; i++) {
            tap = taps[i];
            if(tap->type & OP_SCAN) {
                // Connect via cartesian product
                OpBase *cartesianProduct = NewCartesianProductOp(AST_AliasCount(ast));
                ExecutionPlan_PushBelow(tap, cartesianProduct);
                _OpBase_AddChild(cartesianProduct, a->root);
                break;
            }
        }
    }

    array_free(taps);
    // Null root to avoid freeing connected operations.
    a->root = NULL;

    // Copy connected components over.
    if(a->connected_components) {
        if(!b->connected_components) {
            b->connected_components = a->connected_components;
            a->connected_components = NULL;
        } else {
            uint cc_count = array_len(a->connected_components);
            for(int i = 0; i < cc_count; i++) {
                QueryGraph *cc = array_pop(a->connected_components);
                b->connected_components = array_append(b->connected_components, cc);
            }
        }
    }

    ExecutionPlanFree(a);
    return b;
}

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, AST **ast, ResultSet *result_set, bool explain) {
    ExecutionPlan *plan = NULL;
    ExecutionPlan *curr_plan;

    for(unsigned int i = 0; i < array_len(ast); i++) {
        curr_plan = _NewExecutionPlan(ctx, ast[i], result_set);
        if(i == 0) plan = curr_plan;
        else plan = _ExecutionPlan_Connect(plan, curr_plan, ast[i]);

        if(ast[i]->whereNode != NULL) {
            Vector *sub_trees = FilterTree_SubTrees(curr_plan->filter_tree);

            /* For each filter tree find the earliest position along the execution 
             * after which the filter tree can be applied. */
            for(int j = 0; j < Vector_Size(sub_trees); j++) {
                FT_FilterNode *tree;
                Vector_Get(sub_trees, j, &tree);

                rax *references = FilterTree_CollectAliases(tree);

                /* Scan execution plan, locate the earliest position where all 
                 * references been resolved. */
                OpBase *op = ExecutionPlan_Locate_References(plan->root, references);
                assert(op);

                /* Create filter node.
                 * Introduce filter op right below located op. */
                OpBase *filter_op = NewFilterOp(tree);
                ExecutionPlan_PushBelow(op, filter_op);                
                raxFree(references);
            }
            Vector_Free(sub_trees);
        }
        optimizePlan(plan, ast[i]);
    }

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

void _ExecutionPlanInit(OpBase *root) {
    if(root->init) root->init(root);
    for(int i = 0; i < root->childCount; i++) {
        _ExecutionPlanInit(root->children[i]);
    }
}

void ExecutionPlanInit(ExecutionPlan *plan) {
    if(!plan) return;
    _ExecutionPlanInit(plan->root);
}

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    Record r;
    OpBase *op = plan->root;
    
    ExecutionPlanInit(plan);
    while((r = op->consume(op)) != NULL) Record_Free(r);
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

void _ExecutionPlanFreeRecursive(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanFreeRecursive(op->children[i]);
    }
    OpBase_Free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    if(plan == NULL) return;
    if(plan->root) _ExecutionPlanFreeRecursive(plan->root);
    if(plan->connected_components) {
        for(int i = 0; i < array_len(plan->connected_components); i++) {
            QueryGraph_Free(plan->connected_components[i]);
        }
        array_free(plan->connected_components);
    }
    free(plan);
}

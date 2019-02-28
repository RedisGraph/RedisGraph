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
        parent->children = malloc(sizeof(OpBase *));
    } else {
        parent->children = realloc(parent->children, sizeof(OpBase *) * (parent->childCount+1));
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
        free(parent->children);
        parent->children = NULL;
    } else {
        // Shift left children.
        for(int j = i; j < parent->childCount; j++) {
            parent->children[j] = parent->children[j+1];
        }
        parent->children = realloc(parent->children, sizeof(OpBase *) * parent->childCount);
    }

    // Remove parent from child.
    child->parent = NULL;
}

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
    _OpBase_RemoveNode(parent, child);
}

Vector* _ExecutionPlan_Locate_References(OpBase *root, OpBase **op, Vector *references) {
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
    int match = Vector_Size(references);
    size_t ref_count = Vector_Size(references);
    size_t seen_count = Vector_Size(seen);

    for(int i = 0; i < ref_count; i++) {
        char *reference;        
        Vector_Get(references, i, &reference);

        int j = 0;
        for(; j < seen_count; j++) {
            char *resolved;
            Vector_Get(seen, j, &resolved);
            if(!strcmp(reference, resolved)) {
                /* Match! */
                break;
            }
        }
        
        /* no match, quick break, */
        if (j == seen_count) break;
        else match--;
    }

    if(!match) *op = root;
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
}

OpBase* ExecutionPlan_Locate_References(OpBase *root, Vector *references) {
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

ExecutionPlan* _NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, AST *ast, ResultSet *result_set) {
    Graph *g = gc->g;
    ExecutionPlan *execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));    
    execution_plan->result_set = result_set;
    Vector *ops = NewVector(OpBase*, 1);
    OpBase *op;

    /* Predetermin graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    size_t node_count;
    size_t edge_count;
    _Determine_Graph_Size(ast, &node_count, &edge_count);
    QueryGraph *q = QueryGraph_New(node_count, edge_count);
    execution_plan->query_graph = q;
    FT_FilterNode *filter_tree = NULL;
    if(ast->whereNode != NULL) {
        filter_tree = BuildFiltersTree(ast, ast->whereNode->filters);
        execution_plan->filter_tree = filter_tree;
    }

    if(ast->matchNode) {
        BuildQueryGraph(gc, q, ast->matchNode->_mergedPatterns);

        // For every pattern in match clause.
        size_t patternCount = Vector_Size(ast->matchNode->patterns);
        
        /* Incase we're dealing with multiple patterns
         * we'll simply join them all together with a join operation. */
        bool multiPattern = patternCount > 1;
        OpBase *cartesianProduct = NULL;
        if(multiPattern) {
            cartesianProduct = NewCartesianProductOp(AST_AliasCount(ast));
            Vector_Push(ops, cartesianProduct);
        }
        
        // Keep track after all traversal operations along a pattern.
        Vector *traversals = NewVector(OpBase*, 1);

        for(int i = 0; i < patternCount; i++) {
            Vector *pattern;
            Vector_Get(ast->matchNode->patterns, i, &pattern);

            if(Vector_Size(pattern) > 1) {
                size_t expCount = 0;
                AlgebraicExpression **exps = AlgebraicExpression_From_Query(ast, pattern, q, &expCount);

                TRAVERSE_ORDER order = determineTraverseOrder(filter_tree, exps, expCount);
                if(order == TRAVERSE_ORDER_FIRST) {
                    AlgebraicExpression *exp = exps[0];
                    selectEntryPoint(exp, filter_tree);

                    // Create SCAN operation.
                    if(exp->src_node->label) {
                        /* There's no longer need for the last matrix operand
                         * as it's been replaced by label scan. */
                        AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, NULL);
                        op = NewNodeByLabelScanOp(gc, exp->src_node, ast);
                        Vector_Push(traversals, op);
                    } else {
                        op = NewAllNodeScanOp(g, exp->src_node, ast);
                        Vector_Push(traversals, op);
                    }
                    for(int i = 0; i < expCount; i++) {
                        if(exps[i]->operand_count == 0) continue;
                        if(exps[i]->edgeLength) {
                            op = NewCondVarLenTraverseOp(exps[i],
                                                         exps[i]->edgeLength->minHops,
                                                         exps[i]->edgeLength->maxHops,
                                                         g,
                                                         ast);
                        }
                        else {
                            op = NewCondTraverseOp(g, exps[i], ast);
                        }
                        Vector_Push(traversals, op);
                    }
                } else {
                    AlgebraicExpression *exp = exps[expCount-1];
                    selectEntryPoint(exp, filter_tree);
                    // Create SCAN operation.
                    if(exp->dest_node->label) {
                        /* There's no longer need for the last matrix operand
                         * as it's been replaced by label scan. */
                        AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, NULL);
                        op = NewNodeByLabelScanOp(gc, exp->dest_node, ast);
                        Vector_Push(traversals, op);
                    } else {
                        op = NewAllNodeScanOp(g, exp->dest_node, ast);
                        Vector_Push(traversals, op);
                    }

                    for(int i = expCount-1; i >= 0; i--) {
                        if(exps[i]->operand_count == 0) continue;
                        AlgebraicExpression_Transpose(exps[i]);
                        if(exps[i]->edgeLength) {
                            op = NewCondVarLenTraverseOp(exps[i],
                                                         exps[i]->edgeLength->minHops,
                                                         exps[i]->edgeLength->maxHops,
                                                         g,
                                                         ast);
                        }
                        else {
                            op = NewCondTraverseOp(g, exps[i], ast);
                        }
                        Vector_Push(traversals, op);
                    }
                }
            } else {
                /* Node scan. */
                AST_GraphEntity *ge;
                Vector_Get(pattern, 0, &ge);
                Node **n = QueryGraph_GetNodeRef(q, QueryGraph_GetNodeByAlias(q, ge->alias));
                if(ge->label)
                    op = NewNodeByLabelScanOp(gc, *n, ast);
                else
                    op = NewAllNodeScanOp(g, *n, ast);
                Vector_Push(traversals, op);
            }
            
            if(multiPattern) {
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
    }

    /* Set root operation */
    if(ast->createNode) {
        BuildQueryGraph(gc, q, ast->createNode->graphEntities);
        OpBase *opCreate = NewCreateOp(ctx, gc, ast, q, execution_plan->result_set);

        Vector_Push(ops, opCreate);
    }

    if(ast->mergeNode) {
        OpBase *opMerge = NewMergeOp(gc, ast, execution_plan->result_set);
        Vector_Push(ops, opMerge);
    }

    if(ast->deleteNode) {
        OpBase *opDelete = NewDeleteOp(ast->deleteNode, q, gc, execution_plan->result_set, ast);
        Vector_Push(ops, opDelete);
    }

    if(ast->setNode) {
        OpBase *op_update = NewUpdateOp(gc, ast, execution_plan->result_set);
        Vector_Push(ops, op_update);
    }

    if(ast->skipNode && !ast->orderNode) {
        OpBase *op_skip = NewSkipOp(ast->skipNode->skip);
        Vector_Push(ops, op_skip);
    }

    if(ast->limitNode && !ast->orderNode) {
        OpBase *op_limit = NewLimitOp(ast->limitNode->limit);
        Vector_Push(ops, op_limit);
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

    if(ast->orderNode) {
        op = NewSortOp(ast, _OrderClause_GetExpressions(ast));
        Vector_Push(ops, op);
    }

    if(ast->skipNode && ast->orderNode) {
        OpBase *op_skip = NewSkipOp(ast->skipNode->skip);
        Vector_Push(ops, op_skip);
    }

    if(ast->limitNode && ast->orderNode) {
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
    // null root to avoid freeing connected operations.
    a->root = NULL;
    // null query graph to avoid freeing entities referenced in both graphs
    // TODO memory leak on entities that exclusively appear in this graph
    free(a->query_graph);
    a->query_graph = NULL;
    ExecutionPlanFree(a);
    return b;
}

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, AST **ast, bool explain) {
    ExecutionPlan *plan = NULL;
    ExecutionPlan *curr_plan;
    
    // Use the last AST, as it is supposed to be the only AST with a RETURN node.
    ExpandCollapsedNodes(ast[array_len(ast)-1]);
    ResultSet *result_set = NULL;
    if(!explain) {
        result_set = NewResultSet(ast[array_len(ast)-1], ctx);
        ResultSet_CreateHeader(result_set, ast[array_len(ast)-1]);
    }

    for(int i = 0; i < array_len(ast); i++) {
        curr_plan = _NewExecutionPlan(ctx, gc, ast[i], result_set);
        if(i == 0) plan = curr_plan;
        else plan = _ExecutionPlan_Connect(plan, curr_plan, ast[i]);

        if(ast[i]->whereNode != NULL) {
            Vector *sub_trees = FilterTree_SubTrees(curr_plan->filter_tree);

            /* For each filter tree find the earliest position along the execution 
            * after which the filter tree can be applied. */
            for(int i = 0; i < Vector_Size(sub_trees); i++) {
                FT_FilterNode *tree;
                Vector_Get(sub_trees, i, &tree);

                Vector *references = FilterTree_CollectAliases(tree);

                /* Scan execution plan, locate the earliest position where all 
                 * references been resolved. */
                OpBase *op = ExecutionPlan_Locate_References(plan->root, references);
                assert(op);

                /* Create filter node.
                 * Introduce filter op right below located op. */
                OpBase *filter_op = NewFilterOp(tree);
                ExecutionPlan_PushBelow(op, filter_op);
                for(int j = 0; j < Vector_Size(references); j++) {
                    char *ref;
                    Vector_Get(references, j, &ref);
                    free(ref);
                }
                Vector_Free(references);
            }
            Vector_Free(sub_trees);
        }
        optimizePlan(gc, plan, ast[i]);
    }

    return plan;
}

void _ExecutionPlanPrint(const OpBase *op, char **strPlan, int ident) {
    char strOp[512] = {0};
    sprintf(strOp, "%*s%s\n", ident, "", op->name);
    
    if(*strPlan == NULL) {
        *strPlan = calloc(strlen(strOp) + 1, sizeof(char));
    } else {
        *strPlan = realloc(*strPlan, sizeof(char) * (strlen(*strPlan) + strlen(strOp) + 2));
    }
    strcat(*strPlan, strOp);

    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanPrint(op->children[i], strPlan, ident + 4);
    }
}

char* ExecutionPlanPrint(const ExecutionPlan *plan) {
    char *strPlan = NULL;
    _ExecutionPlanPrint(plan->root, &strPlan, 0);
    return strPlan;
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

void _ExecutionPlanFreeRecursive(OpBase* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanFreeRecursive(op->children[i]);
    }
    OpBase_Free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    if(plan == NULL) return;
    if(plan->root) _ExecutionPlanFreeRecursive(plan->root);

    QueryGraph_Free(plan->query_graph);
    free(plan);
}

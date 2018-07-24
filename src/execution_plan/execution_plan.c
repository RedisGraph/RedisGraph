/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../graph/edge.h"
#include "../rmutil/vector.h"
#include "../query_executor.h"
#include "../arithmetic/algebraic_expression.h"
#include "./optimizations/optimizer.h"
#include "./optimizations/optimizations.h"

/* Forward declarations */
OpResult PullFromStreams(OpNode *source, QueryGraph *graph);

OpNode* NewOpNode(OpBase *op) {
    OpNode *opNode = malloc(sizeof(OpNode));
    opNode->operation = op;
    opNode->children = NULL;
    opNode->childCount = 0;
    opNode->parent = NULL;
    opNode->state = StreamUnInitialized;
    return opNode;
}

/* Checks if parent has given child, if so returns 1
 * otherwise returns 0 */
int _OpNode_ContainsChild(const OpNode *parent, const OpNode *child) {
    for(int i = 0; i < parent->childCount; i++) {
        if(parent->children[i] == child) {
            return 1;
        }
    }
    return 0;
}

void _OpNode_AddChild(OpNode *parent, OpNode *child) {
    // Add child to parent
    if(parent->children == NULL) {
        parent->children = malloc(sizeof(OpNode *));
    } else {
        parent->children = realloc(parent->children, sizeof(OpNode *) * (parent->childCount+1));
    }
    parent->children[parent->childCount++] = child;

    // Add parent to child
    child->parent = parent;
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpNode_RemoveNode(OpNode *parent, OpNode *child) {
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
        parent->children = realloc(parent->children, sizeof(OpNode *) * parent->childCount);
    }

    // Remove parent from child.
    child->parent = NULL;
}

void _OpNode_RemoveChild(OpNode *parent, OpNode *child) {
    _OpNode_RemoveNode(parent, child);
}

/* Push b right below a. */
void _OpNode_PushBelow(OpNode *a, OpNode *b) {
    /* B shouldn't have its parent set. */
    assert(!(b->parent || b->children));
    assert(a->parent);

    /* Replace A's former parent. */
    OpNode *a_former_parent = a->parent;

    /* Disconnect A from its former parent. */
    _OpNode_RemoveChild(a_former_parent, a);

    /* Add A's former parent as parent of B. */
    _OpNode_AddChild(a_former_parent, b);

    /* Add A as a child of B. */
    _OpNode_AddChild(b, a);
}

/* Push b right above a. */
void _OpNode_PushAbove(OpNode *a, OpNode *b) {
    /* B shouldn't have its children set. */
    assert(b->children != NULL);

    /* Remove each child of A and add it as a child of B. */
    while(a->childCount) {
        OpNode* child = a->children[0];
        _OpNode_RemoveChild(a, child);
        _OpNode_AddChild(b, child);
    }

    /* B is the only child of A. */
    _OpNode_AddChild(a, b);
}

void ExecutionPlan_RemoveOp(OpNode *op) {
    assert(op->parent != NULL);
    
    // Remove op from its parent.
    OpNode* parent = op->parent;
    _OpNode_RemoveChild(op->parent, op);

    // Add each of op's children as a child of op's parent.
    for(int i = 0; i < op->childCount; i++) {
        _OpNode_AddChild(parent, op->children[i]);
    }
}

Vector* _ExecutionPlan_Locate_References(OpNode *root, OpNode **op, Vector *references) {
    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * previously modified entities (up the execution plan). */
    Vector *seen = NewVector(char*, 0);
    char *modifiedEntity;

    /* Append current op modified entities. */
    if(root->operation->modifies) {
        for(int i = 0; i < Vector_Size(root->operation->modifies); i++) {
            Vector_Get(root->operation->modifies, i, &modifiedEntity);
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

OpNode* ExecutionPlan_Locate_References(OpNode *root, Vector *references) {
    OpNode *op = NULL;
    Vector *temp = _ExecutionPlan_Locate_References(root, &op, references);
    Vector_Free(temp);
    return op;
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

void _Determine_Graph_Size(const AST_Query *ast, size_t *node_count, size_t *edge_count) {
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

ExecutionPlan* NewExecutionPlan(RedisModuleCtx *ctx, Graph *g,
                                const char *graph_name,
                                AST_Query *ast,
                                bool explain) {

    ExecutionPlan *execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));
    execution_plan->root = NewOpNode(NULL);    
    execution_plan->graph_name = graph_name;
    execution_plan->result_set = (explain) ? NULL: NewResultSet(ast, ctx);
    execution_plan->filter_tree = NULL;
    Vector *ops = NewVector(OpNode*, 1);
    OpNode *op;

    /* Predetermin graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    size_t node_count;
    size_t edge_count;
    _Determine_Graph_Size(ast, &node_count, &edge_count);
    QueryGraph *q = NewQueryGraph_WithCapacity(node_count, edge_count);
    execution_plan->graph = q;
    
    if(ast->whereNode != NULL)
        execution_plan->filter_tree = BuildFiltersTree(ast->whereNode->filters);

    if(ast->matchNode) {
        BuildQueryGraph(ctx, g, graph_name, q, ast->matchNode->_mergedPatterns);

        // For every pattern in match clause.
        size_t patternCount = Vector_Size(ast->matchNode->patterns);
        
        /* Incase we're dealing with multiple patterns
         * we'll simply join them all together with a join operation. */
        bool multiPattern = patternCount > 1;
        OpNode *cartesianProduct = NULL;
        if(multiPattern) {
            cartesianProduct = NewOpNode(NewCartesianProductOp());
            Vector_Push(ops, cartesianProduct);
        }
        
        // Keep track after all traversal operations along a pattern.
        Vector *traversals = NewVector(OpNode*, 1);

        for(int i = 0; i < patternCount; i++) {
            Vector *pattern;
            Vector_Get(ast->matchNode->patterns, i, &pattern);

            if(Vector_Size(pattern) > 1) {
                size_t expCount = 0;
                AlgebraicExpression **exps = AlgebraicExpression_From_Query(ast, pattern, q, &expCount);
                
                TRAVERSE_ORDER order = determineTraverseOrder(q, execution_plan->filter_tree, exps, expCount);
                if(order == TRAVERSE_ORDER_LAST) {
                    op = NewOpNode(NewTraverseOp(g, q, exps[expCount-1]));
                    Vector_Push(traversals, op);
                    for(int i = expCount-2; i >= 0; i--) {
                        op = NewOpNode(NewCondTraverseOp(g, q, exps[i]));
                        Vector_Push(traversals, op);
                    }
                } else {
                    AlgebraicExpression_Transpose(exps[0]);
                    op = NewOpNode(NewTraverseOp(g, q, exps[0]));
                    Vector_Push(traversals, op);
                    for(int i = 1; i < expCount; i++) {
                        AlgebraicExpression_Transpose(exps[i]);
                        op = NewOpNode(NewCondTraverseOp(g, q, exps[i]));
                        Vector_Push(traversals, op);
                    }
                }
            } else {
                /* Node scan. */
                AST_GraphEntity *ge;
                Vector_Get(pattern, 0, &ge);
                Node **n = QueryGraph_GetNodeRef(q, QueryGraph_GetNodeByAlias(q, ge->alias));
                if(ge->label)
                    op = NewOpNode(NewNodeByLabelScanOp(ctx, q, g, graph_name, n, ge->label));
                else
                    op = NewOpNode(NewAllNodeScanOp(q, g, n));
                Vector_Push(traversals, op);
            }
            
            if(multiPattern) {
                // Connect traversal operations.
                OpNode *childOp;
                OpNode *parentOp;
                Vector_Pop(traversals, &parentOp);
                // Connect cartesian product to the root of traversal.
                _OpNode_AddChild(cartesianProduct, parentOp);
                while(Vector_Pop(traversals, &childOp)) {
                    _OpNode_AddChild(parentOp, childOp);
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

    /* Set root operation */
    if(ast->createNode) {
        BuildQueryGraph(ctx, g, graph_name, q, ast->createNode->graphEntities);

        int request_refresh = (ast->matchNode != NULL);
        OpNode *opCreate = NewOpNode(NewCreateOp(ctx, ast, g, q, graph_name, request_refresh,
                                        execution_plan->result_set));

        Vector_Push(ops, opCreate);
    }

    if(ast->mergeNode) {
        OpNode *opMerge = NewOpNode(NewMergeOp(ctx, ast, g, q, graph_name, execution_plan->result_set));
        Vector_Push(ops, opMerge);
    }

    if(ast->deleteNode) {
        OpNode *opDelete = NewOpNode(NewDeleteOp(ast->deleteNode, q, g, execution_plan->result_set));
        Vector_Push(ops, opDelete);
    }

    if(ast->setNode) {
        OpNode *op_update = NewOpNode(NewUpdateOp(ctx, ast, q, execution_plan->result_set, graph_name));
        Vector_Push(ops, op_update);
    }

    if(ast->returnNode) {
        if(ReturnClause_ContainsAggregation(ast->returnNode)) {
           op = NewOpNode(NewAggregateOp(ctx, ast));
        } else {
            op = NewOpNode(NewProduceResultsOp(ast, execution_plan->result_set));
        }
        Vector_Push(ops, op);
    }

    OpNode *parent_op;
    OpNode *child_op;
    Vector_Pop(ops, &parent_op);
    execution_plan->root = parent_op;

    while(Vector_Pop(ops, &child_op)) {
        _OpNode_AddChild(parent_op, child_op);
        parent_op = child_op;
    }

    if(ast->whereNode != NULL) {
        Vector *sub_trees = FilterTree_SubTrees(execution_plan->filter_tree);

        /* For each filter tree find the earliest position along the execution 
         * after which the filter tree can be applied. */
        for(int i = 0; i < Vector_Size(sub_trees); i++) {
            FT_FilterNode *tree;
            Vector_Get(sub_trees, i, &tree);

            Vector *references = FilterTree_CollectAliases(tree);

            /* Scan execution plan, locate the earliest position where all 
             * references been resolved. */
            OpNode *op = ExecutionPlan_Locate_References(execution_plan->root, references);
            assert(op);

            /* Create filter node.
             * Introduce filter op right below located op. */
            OpNode *filter_op = NewOpNode(NewFilterOp(tree, q));
            _OpNode_PushBelow(op, filter_op);
            for(int j = 0; j < Vector_Size(references); j++) {
                char *ref;
                Vector_Get(references, j, &ref);
                free(ref);
            }
            Vector_Free(references);

            ExecutionPlanPrint(execution_plan);
        }
        Vector_Free(sub_trees);

        /* Optimization rule:
         * two following filter operations
         * might be combined into a single filter operation
         * if their root is not an OR.
         * apply this rule to reduce the number of filter operations. */
    }
    
    if(!explain) optimizePlan(execution_plan);
    return execution_plan;
}

void _ExecutionPlanPrint(const OpNode *op, char **strPlan, int ident) {
    char strOp[512] = {0};
    sprintf(strOp, "%*s%s\n", ident, "", op->operation->name);
    
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

void ResetStream(OpNode *stream) {
    stream->operation->reset(stream->operation);
    
    for(int i = 0; i < stream->childCount; i++) {
        ResetStream(stream->children[i]);
    }
}

OpResult _ExecuteOpNode(OpNode *node, QueryGraph *graph) {
consume:
    node->state = StreamConsuming;
    OpResult res = node->operation->consume(node->operation, graph);
    
    /* Incase we're depleted or require data renewal
     * try to get new data from children */
    if(res == OP_REFRESH) {
        if(node->operation->reset(node->operation) != OP_OK) {
            return OP_ERR;
        }
        
        res = PullFromStreams(node, graph);
        if(res == OP_OK) {
            // We're good to go.
            goto consume;
        }
    }

    return res;
}

OpResult PullFromStreams(OpNode *source, QueryGraph *graph) {
    if(source->childCount == 0) {
        return OP_DEPLETED;
    }

    /* Assuming stream are independent, and do not effect each other. */
    OpNode *stream;

    /* Advance stream(s) */
    int stream_idx = 0;
    for(; stream_idx < source->childCount; stream_idx++) {
        stream = source->children[stream_idx];
        if(_ExecuteOpNode(stream, graph) == OP_OK) {
            break;
        }
    }

    /* All streams are depleted. */
    if(stream_idx == source->childCount) {
        return OP_DEPLETED;
    }

    /* Pull from all uninitialized streams. */
    for(int i = stream_idx+1; i < source->childCount; i++) {
        stream = source->children[i];
        if(stream->state == StreamUnInitialized) {
            if(_ExecuteOpNode(stream, graph) != OP_OK) {
                /* Uninitialized stream failed to provide data. */
                return OP_DEPLETED;
            }
        }
    }

    /* Reset and pull from depleted streams [0, (stream_id-1)]. */
    stream_idx--;
    for(; stream_idx >= 0; stream_idx--) {
        stream = source->children[stream_idx];
        ResetStream(stream);
        if(_ExecuteOpNode(stream, graph) != OP_OK) {
            return OP_ERR;
        }
    }
    
    return OP_OK;
}

ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan) {
    while(_ExecuteOpNode(plan->root, plan->graph) == OP_OK);
    return plan->result_set;
}

void OpNode_Free(OpNode* op) {
    // Free internal operation
    op->operation->free(op->operation);
    free(op->children);
    free(op);
}

void _ExecutionPlanFreeRecursive(OpNode* op) {
    for(int i = 0; i < op->childCount; i++) {
        _ExecutionPlanFreeRecursive(op->children[i]);
    }
    OpNode_Free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    _ExecutionPlanFreeRecursive(plan->root);
    // QueryGraph_Free(plan->graph);
    if(plan->filter_tree)
        FilterTree_Free(plan->filter_tree);
    free(plan);
}

#include <assert.h>

#include "execution_plan.h"
#include "../query_executor.h"

#include "./ops/op_expand_all.h"
#include "./ops/op_expand_into.h"
#include "./ops/op_all_node_scan.h"
#include "./ops/op_node_by_label_scan.h"
#include "./ops/op_produce_results.h"
#include "./ops/op_filter.h"
#include "./ops/op_aggregate.h"
#include "./ops/op_create.h"
#include "./ops/op_delete.h"

#include "../graph/edge.h"
#include "../rmutil/vector.h"

/* Forward declarations */
OpResult PullFromStreams(OpNode *source, Graph *graph);

OpNode* NewOpNode(OpBase *op) {
    OpNode *opNode = malloc(sizeof(OpNode));
    opNode->operation = op;
    opNode->children = NULL;
    opNode->childCount = 0;
    opNode->parents = NULL;
    opNode->parentCount = 0;
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
    }

    parent->children[parent->childCount] = child;
    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(OpNode *) * (parent->childCount+1));

    // Add parent to child
    if(child->parents == NULL) {
        child->parents = malloc(sizeof(OpNode *));
    }

    child->parents[child->parentCount] = parent;
    child->parentCount++;
    child->parents = realloc(child->parents, sizeof(OpNode *) * (child->parentCount+1));
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpNode_RemoveNode(OpNode *a, OpNode *b) {
    // remove child from parent
    for(int i = 0; i < a->childCount; i++) {
        if(a->children[i] == b) {
            // shift left children
            for(int j = i; j < a->childCount-1; j++) {
                a->children[j] = a->children[j+1];
            }
            // uppdate child count
            a->childCount--;
            a->children = realloc(a->children, sizeof(OpNode *) * a->childCount);
            break;
        }
    }

    // remove parent from child
    for(int i = 0; i < b->parentCount; i++) {
        if(b->parents[i] == a) {
            // shift left parents
            for(int j = i; j < b->parentCount-1; j++) {
                b->parents[j] = b->parents[j+1];
            }
            // uppdate parent count
            b->parentCount--;
            b->parents = realloc(b->parents, sizeof(OpNode *) * b->parentCount);
            break;
        }
    }
}

void _OpNode_RemoveChild(OpNode *parent, OpNode *child) {
    _OpNode_RemoveNode(parent, child);
}

void _OpNode_RemoveParent(OpNode *child, OpNode *parent) {
    _OpNode_RemoveNode(parent, child);
}

void _OpNode_PushInBetween(OpNode *parent, OpNode *onlyChild) {
    /* Disconnect every child from parent
     * Add each pareent's child to only child. */
    while(parent->childCount != 0) { 
        _OpNode_AddChild(onlyChild, parent->children[0]);
        _OpNode_RemoveChild(parent, parent->children[0]);
    }

    _OpNode_AddChild(parent, onlyChild);
}

/*  Nodes with more than one incoming edge
 * will take part in two expand operations
 * MergeNodes will replace one of the expand operation
 * with an expand_into operation 
 * plan - execution plan to be modified
 * n - node with two incoming edges. */
void _ExecutionPlan_MergeNodes(ExecutionPlan *plan, const Node *n) {
    if(Node_IncomeDegree(n) != 2) {
        return;
    }

    /* Locate both expand operations involving 
     * n as the dest node. */
    
    OpNode *a = NULL;
    OpNode *b = NULL;

    OpNode *current = NULL;
    Vector *nodesToVisit = NewVector(OpNode*, 3);
    Vector_Push(nodesToVisit, plan->root);
    /* Due to the structure of our execution plan
     * there's not need to maintain a visited nodes 
     * as in classic BFS/DFS. */
    while(Vector_Size (nodesToVisit) > 0) {
        Vector_Pop(nodesToVisit, &current);
        if(current->operation->type == OPType_EXPAND_ALL) {
            ExpandAll *op = (ExpandAll*)current->operation;
            // if(Node_Compare(*(op->dest_node), n)) {
                if(*op->dest_node == n) {
                if(a == NULL) {
                    a = current;
                    continue;
                } else {
                    b = current;
                    break;
                }
            }
        }

        for(int i = 0; i < current->childCount; i++) {
            Vector_Push(nodesToVisit, current->children[i]);
        }
    }
    Vector_Free(nodesToVisit);
    
    if(a == NULL || b == NULL) {
        return;
    }

    /* now that both operations involving node n
     * are found we can replace one of them with
     * an expand into operation */
    ExpandAll *op = (ExpandAll*)a->operation;
    OpBase *opExpandInto;
    NewExpandIntoOp(op->ctx, plan->graph, plan->graph_name, op->src_node,
                    op->relation, op->dest_node, &opExpandInto);

    // free previous operation.
    a->operation->free(a->operation);
    a->operation = opExpandInto;
    OpNode *expandInto = a;

    // link b operation with expand into
    _OpNode_AddChild(expandInto, b);

    // expand into should inherit b's parents
    for(int i = 0; i < b->parentCount; i++) {
        OpNode *bParent = b->parents[i];
        
        if(bParent == expandInto) {
            continue;
        }

        if(!_OpNode_ContainsChild(bParent, expandInto)) {
            _OpNode_AddChild(bParent, expandInto);
        }

        _OpNode_RemoveChild(bParent, b);
    }
}

/* Returns the number of expected IDs given node will generate */
int _ExecutionPlan_EstimateNodeCardinality(RedisModuleCtx *ctx, const char *graph, const Node *n) {
    Store *s = GetStore(ctx, STORE_NODE, graph, n->label);
    return s->cardinality;
}

/* Locates expand all operations which do not have a child operation,
 * And adds a scan operation as a new child. */
void _ExecutionPlan_OptimizeEntryPoints(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                                        AST_QueryExpressionNode *ast, OpNode *root) {
    /* We've reached a leaf. */
    if(root->childCount == 0 && root->operation->type == OPType_EXPAND_ALL) {
        Node **src = ((ExpandAll*)(root->operation))->src_node;
        Node **dest = ((ExpandAll*)(root->operation))->dest_node;
        Node **entry_point = src;

        // Determin which node should be scaned, based on node cardinality
        // int src_cardinality = _ExecutionPlan_EstimateNodeCardinality(ctx, graph_name, *src);
        // int dest_cardinality = _ExecutionPlan_EstimateNodeCardinality(ctx, graph_name, *dest);
        
        // if(dest_cardinality < src_cardinality) {
        //     entry_point = dest;
        // }
        
        OpBase *scan_op = NULL;
        if((*entry_point)->label) {
            /* TODO: when indexing is enabled, use index when possible. */
            scan_op = NewNodeByLabelScanOp(ctx, g, entry_point, graph_name, (*entry_point)->label);
        } else {
            /* Node is not labeled, no other option but a full scan. */
            scan_op = NewAllNodeScanOp(ctx, g, entry_point, graph_name);
        }        
        
        _OpNode_AddChild(root, NewOpNode(scan_op));

    } else {
        /* Continue scanning. */
        for(int i = 0; i < root->childCount; i++) {
            _ExecutionPlan_OptimizeEntryPoints(ctx, g, graph_name, ast, root->children[i]);
        }
    }
}

Vector* _ExecutionPlan_AddFilters(OpNode *root, FT_FilterNode **filterTree) {
    /* We've reached the end of our execution plan. */
    if(root == NULL) {
        return NULL;
    }

    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * previously modified entities (up the execution plan). */
    Vector *seen = NewVector(char*, 0);
    char *modifiedEntity;

    /* Traverse execution plan, upwards. */
    for(int i = root->childCount-1; i >= 0; i--) {
        Vector *saw = _ExecutionPlan_AddFilters(root->children[i], filterTree);
        
        /* No need to continue, filter tree is empty. */
        if(*filterTree == NULL) {
            if(saw != NULL) {
                Vector_Free(saw);
            }
            Vector_Free(seen);
            return NULL;
        }

        /* Add modified entities from previous operation. */
        if(saw != NULL) {
            for(int i = 0; i < Vector_Size(saw); i++) {
                Vector_Get(saw, i, &modifiedEntity);
                Vector_Push(seen, modifiedEntity);
            }
            Vector_Free(saw);
        }
    }

    // See if filter tree filters any of the current op modified entities
    if(FilterTree_ContainsNode(*filterTree, seen)) {    
        // Create a minimum filter tree for the current execution plan operation
        FT_FilterNode *minTree = FilterTree_MinFilterTree(*filterTree, seen);
        
        // Remove op modified entities from main filter tree.
        FilterTree_RemovePredNodes(filterTree, seen);
        
        OpNode *nodeFilter = NewOpNode(NewFilterOp(minTree));
        _OpNode_PushInBetween(root, nodeFilter);
    }

    /* Append current op modified entities. */
    if(root->operation->modifies) {
        for(int i = 0; i < Vector_Size(root->operation->modifies); i++) {
            Vector_Get(root->operation->modifies, i, &modifiedEntity);
            Vector_Push(seen, modifiedEntity);
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

void _Determine_Graph_Size(const AST_QueryExpressionNode *ast, size_t *node_count, size_t *edge_count) {
    *edge_count = 0;
    *node_count = 0;
    Vector *entities;
    
    if(ast->matchNode) {
        entities = ast->matchNode->graphEntities;
        _Count_Graph_Entities(entities, node_count,edge_count);
    }

    if(ast->createNode) {
        entities = ast->createNode->graphEntities;
        _Count_Graph_Entities(entities, node_count,edge_count);
    }
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, const char *graph_name, AST_QueryExpressionNode *ast) {
    /* Predetermin graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old refrences in ExpandAll. */

    size_t node_count;
    size_t edge_count;
    _Determine_Graph_Size(ast, &node_count, &edge_count);
    Graph *graph = NewGraph_WithCapacity(node_count, edge_count);

    if(ast->matchNode) BuildGraph(graph, ast->matchNode->graphEntities);

    ExecutionPlan *execution_plan = (ExecutionPlan*)calloc(1, sizeof(ExecutionPlan));
    
    /* List of operations. */    
    FT_FilterNode *filterTree = NULL;

    Vector *Ops = NewVector(OpNode*, 0);

    execution_plan->root = NewOpNode(NULL);
    execution_plan->graph = graph;
    execution_plan->graph_name = graph_name;
    execution_plan->result_set = NewResultSet(ast);

    Vector_Push(Ops, execution_plan->root);

    /* Get all nodes without incoming edges */
    Vector *entryNodes = Graph_GetNDegreeNodes(graph, 0);

    for(int i = 0; i < Vector_Size(entryNodes); i++) {
        Node *node;
        Vector_Get(entryNodes, i, &node);
        
        /* Advance if possible. */
        if(Vector_Size(node->outgoing_edges) > 0) {
            Vector *reversedExpandOps = NewVector(OpNode*, 0);

            /* Traverse sub-graph expanded from current node. */
            Node *srcNode = node;
            Node *destNode;
            Edge *edge;

            while(Vector_Size(srcNode->outgoing_edges) > 0) {
                Vector_Get(srcNode->outgoing_edges, 0, &edge);
                destNode = edge->dest;
                
                OpNode *opNodeExpandAll = NewOpNode(NewExpandAllOp(ctx, graph, graph_name,
                                                                   Graph_GetNodeRef(graph, srcNode),
                                                                   Graph_GetEdgeRef(graph, edge),
                                                                   Graph_GetNodeRef(graph, destNode)));
                Vector_Push(reversedExpandOps, opNodeExpandAll);
                
                /* Advance. */
                srcNode = destNode;
            }

            /* Save expand ops in reverse order. */
            while(Vector_Size(reversedExpandOps) > 0) {
                OpNode *opNodeExpandAll;
                Vector_Pop(reversedExpandOps, &opNodeExpandAll);
                Vector_Push(Ops, opNodeExpandAll);
            }
        } else {
            /* Node doesn't have any incoming nor outgoing edges, 
             * this is an hanging node "()", create a scan operation. */
            OpNode *scan_op;
            if(node->label) {
                /* TODO: when indexing is enabled, use index when possible. */
                scan_op = NewOpNode(NewNodeByLabelScanOp(ctx, graph, Graph_GetNodeRef(graph, node),
                                    graph_name, node->label));
            } else {
                /* Node is not labeled, no other option but a full scan. */
                scan_op = NewOpNode(NewAllNodeScanOp(ctx, graph, Graph_GetNodeRef(graph, node),
                                    graph_name));
            }
            Vector_Push(Ops, scan_op);
        }

        /* Consume Ops in reversed order. */
        if(Vector_Size(Ops) > 1) {
            OpNode* currentOp;
            OpNode* prevOp;
            Vector_Pop(Ops, &prevOp);
            
            /* Connect operations in reversed order */
            do {
                Vector_Pop(Ops, &currentOp);
                _OpNode_AddChild(currentOp, prevOp);
                prevOp = currentOp;
            } while(Vector_Size(Ops) != 0);

            /* Reintroduce root. */
            Vector_Push(Ops, execution_plan->root);
        }
    } /* End of entry nodes loop */

    Vector_Free(Ops);

    /* Set root operation */
    if(ast->deleteNode) {
        execution_plan->root->operation = NewDeleteOp(ctx, ast->deleteNode, graph,
                                                      graph_name, execution_plan->result_set);
    } else {
        if(ast->returnNode) {
            if(execution_plan->result_set->aggregated) {
                execution_plan->root->operation = NewAggregateOp(ctx, ast);
            } else {
                execution_plan->root->operation = NewProduceResultsOp(ctx, ast,
                                                                      execution_plan->result_set);
            }
        }

        if(ast->createNode) {
            BuildGraph(graph, ast->createNode->graphEntities);
            int request_refresh = (execution_plan->root->childCount > 0);
            OpBase *op_create = NewCreateOp(ctx, graph, graph_name, request_refresh,
                                            execution_plan->result_set);
            
            if(execution_plan->root->operation == NULL) {
                /* Set root operation to create. */
                execution_plan->root->operation = op_create;
            } else {
                /* Push create operation between root and its children. */
                _OpNode_PushInBetween(execution_plan->root, NewOpNode(op_create));
            }
        }
    }

    /* Optimizations and modifications. */
    _ExecutionPlan_OptimizeEntryPoints(ctx, graph, graph_name, ast, execution_plan->root);
    
    Vector *nodesToMerge = Graph_GetNDegreeNodes(graph, 2);
    for(int i = 0; i < Vector_Size(nodesToMerge); i++) {
        Node *nodeToMerge;
        Vector_Get(nodesToMerge, i, & nodeToMerge);
        _ExecutionPlan_MergeNodes(execution_plan, nodeToMerge);
    }

    if(ast->whereNode != NULL) {
        execution_plan->filter_tree = BuildFiltersTree(ast->whereNode->filters);
        _ExecutionPlan_AddFilters(execution_plan->root, &execution_plan->filter_tree);
    }

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

OpResult _ExecuteOpNode(OpNode *node, Graph *graph) {
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

OpResult PullFromStreams(OpNode *source, Graph *graph) {
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
    // Free child operations
    for(int i = 0; i < op->childCount; i++) {
        OpNode_Free(op->children[i]);
    }
    
    // Free internal operation
    op->operation->free(op->operation);
    free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    OpNode_Free(plan->root);
    // Graph_Free(plan->graph);
    // if(plan->filter_tree) {
    //     FilterTree_Free(plan->filter_tree);
    // }
    free(plan);
}
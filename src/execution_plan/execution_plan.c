#include "execution_plan.h"
#include "../query_executor.h"

#include "./ops/expend_all.h"
#include "./ops/all_node_scan.h"
#include "./ops/node_by_label_scan.h"

#include "../graph/edge.h"
#include "../rmutil/vector.h"

OpNode* NewOpNode(OpBase *op) {
    OpNode *opNode = malloc(sizeof(OpNode));
    opNode->operation = op;
    opNode->children = NULL;
    opNode->childCount = 0;
    return opNode;
}

void OpNode_AddChild(OpNode* parent, OpNode* child) {
    if(parent->children == NULL) {
        parent->children = malloc(sizeof(OpNode *));
    }

    parent->children[parent->childCount] = child;
    parent->childCount++;
    parent->children = realloc(parent->children, sizeof(OpNode *) * (parent->childCount+1));
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, RedisModuleString *graphName, QueryExpressionNode *ast) {
    ExecutionPlan *executionPlan = malloc(sizeof(ExecutionPlan));
    
    OpBase *produceResults;
    NewProduceResultsOp(ctx, graphName, ast, &produceResults);
    OpNode *opProduceResults = NewOpNode(produceResults);
    executionPlan->root = opProduceResults;

    // Build graph from AST MATCH clause
    Graph *graph = BuildGraph(ast->matchNode);
    executionPlan->graph = graph;

    // Get all nodes without incoming edges
    Vector *entryNodes = Graph_GetNDegreeNodes(graph, 0);

    for(int i = 0; i < Vector_Size(entryNodes); i++) {
        Node *node;
        Vector_Get(entryNodes, i, &node);

        // List of operations
        Vector *Ops = NewVector(OpNode*, 0);
        
        // Locate node within MATCH clause.
        ChainElement *chainElement;
        for(int j = 0; j < Vector_Size(ast->matchNode->chainElements); j++) {
            Vector_Get(ast->matchNode->chainElements, j, &chainElement);
            if(chainElement->t == N_ENTITY && strcmp(node->alias, chainElement->e.alias) == 0) { break; }
        }

        OpBase* op;
        // Determin operation type
        if(chainElement->e.label == NULL) {
            // Node is not labeled, no other option but a full scan.
            // TODO: if this node is a part of a pattern (A)-[]->(B)
            // then it might be possible to scan B more efficently
            // if B is labeled.
            op = NewAllNodeScanOp(ctx, node, graphName);
        } else {
            // TODO: when indexing is enabled, use index when possible.
            op = NewNodeByLabelScanOp(ctx, node, graphName, chainElement->e.label);
        }
        OpNode *opGetNode = NewOpNode(op);
        
        Vector_Push(Ops, opGetNode);

        // Expend if possible
        if(Vector_Size(node->outgoingEdges) > 0) {
            // Traverse sub-graph expended from current node.
            // Assuming only one edge per node.
            // TODO: this assumption doesn't hold, support multi outgoing edges.
            Node *current = node;
            Node *next;
            Edge *edge;
            Vector_Get(current->outgoingEdges, 0, &edge);
            next = edge->dest;

            while(next != NULL) {
                // Create sub-graph
                // current and next are already connected by an edge.
                Graph *g = NewGraph();
                Graph_AddNode(g, current);
                Graph_AddNode(g, next);

                OpNode *opNodeExpendAll = NewOpNode(NewExpendAllOp(ctx, graphName, g));
                Vector_Push(Ops, opNodeExpendAll);
                
                // can we advance?
                if(Vector_Size(next->outgoingEdges) == 0) {
                    break;
                }
                
                // Advance
                current = next;
                Vector_Get(current->outgoingEdges, 0, &edge);
                next = edge->dest;
            }
        } // End of node expansion

        // Consume Ops in reversed order.
        OpNode* prevPrev;
        OpNode* prev;

        // Connect result-set to last expention
        Vector_Pop(Ops, &prev);
        OpNode_AddChild(opProduceResults, prev);
        
        // Connect operations in reversed order
        while(Vector_Size(Ops) != 0) {
            Vector_Pop(Ops, &prevPrev);
            OpNode_AddChild(prev, prevPrev);
            prev = prevPrev;
        }

        Vector_Free(Ops);
    } // End of entry nodes loop

    return executionPlan;
}

char* _ExecutionPlanPrint(const OpNode *op, char **strPlan, int ident) {
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

OpResult _ExecuteOpNode(OpNode *node, Graph *graph) {
    /* Before current node can execute, all of
     * its children must execute */
    OpResult res = node->operation->next(node->operation, graph);
    
    /* Incase we're depleted or require data renewal
     * try to get new data from children */
    if(OP_REQUIRE_NEW_DATA(res)) {
        if(res == OP_DEPLETED) {
            // Reset on depleted
            if (node->operation->reset(node->operation) == OP_ERR) {
                return OP_ERR;
            }

            if(node->childCount == 0) {
                return node->operation->next(node->operation, graph);
            }
        }

        // Try to get new data into the graph
        for(int i = 0; i < node->childCount; i++) {
            OpNode *child = node->children[i];
            switch(_ExecuteOpNode(child, graph)) {
                case OP_ERR:
                    return OP_ERR;
                case OP_OK:
                    // Managed to get new data into the graph.
                    return node->operation->next(node->operation, graph);
            }
        }
    }
    return res;
}

void ExecutionPlan_Execute(ExecutionPlan *plan) {
    while(_ExecuteOpNode(plan->root, plan->graph) != OP_DEPLETED);
}

void OpNode_Free(OpNode* op) {
    // Free child operations
    for(int i = 0; i < op->childCount; i++) {
        OpNode_Free(op->children[i]);
    }
    free(op);
}

void ExecutionPlanFree(ExecutionPlan *plan) {
    OpNode_Free(plan->root);
    Graph_Free(plan->graph);
    free(plan);
}
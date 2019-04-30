/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "algebraic_expression.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../../deps/rax/rax.h"

#include <assert.h>

AlgebraicExpression *_AE_MUL(size_t operand_cap) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = AL_EXP_MUL;
    ae->operand_cap = operand_cap;
    ae->operand_count = 0;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    ae->edge = NULL;
    ae->edgeLength = NULL;
    return ae;
}

static int _intermidate_node(const Node *n) {
    /* ->()<- 
     * <-()->
     * ->()->
     * <-()<- */
    return ((array_len(n->incoming_edges) > 1) ||
            (array_len(n->outgoing_edges) > 1) ||
            ((array_len(n->incoming_edges) > 0) && (array_len(n->outgoing_edges) > 0)));
}

static int _referred_entity(char *alias, TrieMap *ref_entities) {
    return TRIEMAP_NOTFOUND != TrieMap_Find(ref_entities, alias, strlen(alias));
}

static int _referred_node(const Node *n, TrieMap *ref_entities) {
    return _referred_entity(n->alias, ref_entities);
}

/* For every referenced edge, add edge source and destination nodes
 * as referenced entities. */
static void _referred_edge_ends(TrieMap *ref_entities, const QueryGraph *q) {
    char *alias;
    tm_len_t len;
    void *value;
    Vector *aliases = NewVector(char*, 0);
    TrieMapIterator *it = TrieMap_Iterate(ref_entities, "", 0);

    /* Scan ref_entities for referenced edges.
     * note, we can not modify triemap which scanning it. */
    while(TrieMapIterator_Next(it, &alias, &len, &value)) {
        Edge *e = QueryGraph_GetEdgeByAlias(q, alias);
        if(!e) continue;

        // Remember edge ends.        
        Vector_Push(aliases, e->src->alias);
        Vector_Push(aliases, e->dest->alias);
    }

    // Add edges ends as referenced entities.
    for(int i = 0; i < Vector_Size(aliases); i++) {
        Vector_Get(aliases, i, &alias);
        TrieMap_Add(ref_entities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }

    TrieMapIterator_Free(it);
    Vector_Free(aliases);
}

/* Variable length edges require their own algebraic expression,
 * therefor mark both variable length edge ends as referenced. */
static void _referred_variable_length_edges(TrieMap *ref_entities, Vector *matchPattern, const QueryGraph *q) {
    Edge *e;
    AST_GraphEntity *match_element;

    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        Vector_Get(matchPattern, i, &match_element);
        if(match_element->t != N_LINK) continue;
        
        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        if(!edge->length) continue;

        e = QueryGraph_GetEdgeByAlias(q, edge->ge.alias);
        TrieMap_Add(ref_entities, e->src->alias, strlen(e->src->alias), NULL, TrieMap_DONT_CARE_REPLACE);
        TrieMap_Add(ref_entities, e->dest->alias, strlen(e->dest->alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }
}

/* Variable length expression must contain only a single operand, the edge being 
 * traversed multiple times, in cases such as (:labelA)-[e*]->(:labelB) both label A and B
 * are applied via a label matrix operand, this function migrates A and B from a
 * variable length expression to other expressions. */
static AlgebraicExpression** _AlgebraicExpression_IsolateVariableLenExps(AlgebraicExpression **expressions, size_t *expCount) {
    /* Return value is a new set of expressions, where each variable length expression
      * is guaranteed to have a single operand, as such in the worst case the number of
      * expressions doubles + 1. */
    AlgebraicExpression **res = malloc(sizeof(AlgebraicExpression*) * (*expCount * 2 + 1));
    size_t newExpCount = 0;

    /* Scan through each expression, locate expression which 
     * have a variable length edge in them. */
    for(size_t expIdx = 0; expIdx < *expCount; expIdx++) {
        AlgebraicExpression *exp = expressions[expIdx];
        if(!exp->edgeLength) {
            res[newExpCount++] = exp;
            continue;
        }

        Edge *e = exp->edge;
        Node *src = exp->src_node;
        Node *dest = exp->dest_node;
        
        // A variable length expression with a labeled source node
        // We only care about the source label matrix, when it comes to
        // the first expression, as in the following expressions
        // src is the destination of the previous expression.
        if(src->mat && expIdx == 0) {
            // Remove src node matrix from expression.
            AlgebraicExpressionOperand op;
            AlgebraicExpression_RemoveTerm(exp, 0, &op);

            /* Create a new expression. */
            AlgebraicExpression *newExp = _AE_MUL(1);
            newExp->src_node = exp->src_node;
            newExp->dest_node = exp->src_node;
            AlgebraicExpression_PrependTerm(newExp, op.operand, op.transpose, op.free);
            res[newExpCount++] = newExp;
        }

        res[newExpCount++] = exp;

        // A variable length expression with a labeled destination node.
        if(dest->mat) {
            // Remove dest node matrix from expression.
            AlgebraicExpressionOperand op;
            AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, &op);

            /* See if dest mat can be prepended to the following expression.
             * If not create a new expression. */            
            if(expIdx < *expCount-1 && !expressions[expIdx+1]->edgeLength) {
                AlgebraicExpression_PrependTerm(expressions[expIdx+1], op.operand, op.transpose, op.free);
            } else {
                AlgebraicExpression *newExp = _AE_MUL(1);
                newExp->src_node = exp->dest_node;
                newExp->dest_node = exp->dest_node;
                AlgebraicExpression_PrependTerm(newExp, op.operand, op.transpose, op.free);
                res[newExpCount++] = newExp;
            }
        }
    }

    *expCount = newExpCount;
    free(expressions);
    return res;
}

/* Break down expression into sub expressions.
 * considering referenced intermidate nodes and edges. */
static AlgebraicExpression** _AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * exp->operand_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    int transpose;     // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;

    TrieMap *ref_entities = NewTrieMap();
    ReturnClause_ReferredEntities(ast->returnNode, ref_entities);
    WithClause_ReferredEntities(ast->withNode, ref_entities);
    CreateClause_ReferredEntities(ast->createNode, ref_entities);
    WhereClause_ReferredEntities(ast->whereNode, ref_entities);
    DeleteClause_ReferredEntities(ast->deleteNode, ref_entities);    
    SetClause_ReferredEntities(ast->setNode, ref_entities);
    _referred_edge_ends(ref_entities, q);
    _referred_variable_length_edges(ref_entities, matchPattern, q);

    AlgebraicExpression *iexp = _AE_MUL(exp->operand_count);
    iexp->src_node = exp->src_node;
    iexp->dest_node = exp->dest_node;
    iexp->operand_count = 0;
    expressions[expIdx++] = iexp;

    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        AST_GraphEntity *match_element;
        Vector_Get(matchPattern, i, &match_element);

        if(match_element->t != N_LINK) continue;

        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        transpose = (edge->direction == N_RIGHT_TO_LEFT);
        e = QueryGraph_GetEdgeByAlias(q, edge->ge.alias);

        /* If edge is referenced, set expression edge pointer. */
        if(_referred_entity(edge->ge.alias, ref_entities))
            iexp->edge = e;

        /* If this is a variable length edge, which is not fixed in length
         * remember edge length. */
        if(edge->length && !AST_LinkEntity_FixedLengthEdge(edge)) {
            iexp->edgeLength = edge->length;
            iexp->edge = e;
        }

        dest = e->dest;
        src = e->src;
        if(transpose) {
            dest = e->src;
            src = e->dest;
        }        

        if(operandIdx == 0 && src->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        unsigned int hops = 1;
        /* Expand fixed variable length edge */
        if(edge->length && AST_LinkEntity_FixedLengthEdge(edge)) {
            hops = edge->length->minHops;
        }

        for(int j = 0; j < hops; j++) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        if(dest->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        /* If intermidate node is referenced, create a new algebraic expression. */
        if(_intermidate_node(dest) && _referred_node(dest, ref_entities)) {
            // Finalize current expression.
            iexp->dest_node = dest;

            /* Create a new algebraic expression. */
            iexp = _AE_MUL(exp->operand_count - operandIdx);
            iexp->operand_count = 0;
            iexp->src_node = expressions[expIdx-1]->dest_node;
            iexp->dest_node = exp->dest_node;
            expressions[expIdx++] = iexp;
        }
    }
    
    *exp_count = expIdx;
    TrieMap_Free(ref_entities, TrieMap_NOP_CB);
    return expressions;
}

static inline void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B, GrB_Descriptor desc) {
    // Using our own compile-time, user defined semiring see rg_structured_bool.m4
    // A,B,C must be boolean matrices.
    GrB_Info res = GrB_mxm(
        C,                  // Output
        GrB_NULL,           // Mask
        GrB_NULL,           // Accumulator
        Rg_structured_bool, // Semiring
        A,                  // First matrix
        B,                  // Second matrix
        desc                // Descriptor        
    );
    assert(res == GrB_SUCCESS);
}

// Reverse order of operand within expression,
// A*B*C will become C*B*A. 
static void _AlgebraicExpression_ReverseOperandOrder(AlgebraicExpression *exp) {
    int right = exp->operand_count-1;
    int left = 0;
    while(right > left) {
        AlgebraicExpressionOperand leftOp = exp->operands[left];
        exp->operands[left] = exp->operands[right];
        exp->operands[right] = leftOp;
        right--;
        left++;
    }
}

void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp, bool freeOp) {
    assert(ae);    
    if(ae->operand_count+1 > ae->operand_cap) {
        ae->operand_cap += 4;
        ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    }

    ae->operands[ae->operand_count].transpose = transposeOp;
    ae->operands[ae->operand_count].free = freeOp;
    ae->operands[ae->operand_count].operand = m;
    ae->operand_count++;
}

void AlgebraicExpression_PrependTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp, bool freeOp) {
    assert(ae);

    ae->operand_count++;
    if(ae->operand_count+1 > ae->operand_cap) {
        ae->operand_cap += 4;
        ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    }

    // TODO: might be optimized with memcpy.
    // Shift operands to the right, making room at the begining.
    for(int i = ae->operand_count-1; i > 0 ; i--) {
        ae->operands[i] = ae->operands[i-1];
    }

    ae->operands[0].transpose = transposeOp;
    ae->operands[0].free = freeOp;
    ae->operands[0].operand = m;
}

// Returns a set of nodes reached at given level from S.
static Node** _BFS(Node *s, uint level) {
    void *seen;                             // Has node been visited?
    uint current_level = 0;                 // Tracks BFS level.
    rax *visited = raxNew();                // Dictionary of visited nodes.
    Node **next = array_new(Node*, 0);      // Nodes to explore next.
    Node **current = array_new(Node*, 1);   // Nodes currently explored.
    
    current = array_append(current, s);
    
    // As long as we've yet to reach required level.
    while(current_level < level) {
        // As long as there are nodes in the frontier.
        while(array_len(current)) {
            Node *n = array_pop(current);

            // Mark n as visited.
            if(!raxInsert(visited, (unsigned char*)n->alias, strlen(n->alias), NULL, NULL)) {
                // We've already processed n.
                continue;
            }

            // Expand node N by visiting all of its neighbors
            for(int i = 0; i < array_len(n->outgoing_edges); i++) {
                Edge *e = n->outgoing_edges[i];
                char *dest = e->dest->alias;
                seen = raxFind(visited, (unsigned char*)dest, strlen(dest));
                if(seen == raxNotFound) next = array_append(next, e->dest);
            }
            for(int i = 0; i < array_len(n->incoming_edges); i++) {
                Edge *e = n->incoming_edges[i];
                char *src = e->src->alias;
                seen = raxFind(visited, (unsigned char*)src, strlen(src));
                if(seen == raxNotFound) next = array_append(next, e->src);
            }
        }

        // Queue consumed, swap queues.
        current_level++;
        if(array_len(next) == 0) break;

        Node **tmp = current;
        current = next;
        next = current;
    }

    array_free(current);
    return next;
}

AlgebraicExpression **AlgebraicExpression_From_QueryGraph(const QueryGraph *g) {
    /* Construct algebraic expression(s) from query graph 
     * trying to take advantage of long multiplications with as few 
     * transpose as possible we'll transform paths crossing the graph 
     * diameter, these are guarantee to be the longest, although
     * there might be situations in which these are not the most optimal paths
     * to explore.
     * 
     * Once a path been transformed it's removed from the graph and the process
     * repeat itself. */

    assert(g);
    AlgebraicExpression **exps;

    // Pick a random node.
    Node *s = g->nodes[0];

    // Get leaf nodes at the deepest level from S.
    Node **leafs = _BFS(s, UINT_MAX);

    exps = array_new(AlgebraicExpression*, 1);

    return exps;
}

AlgebraicExpression **AlgebraicExpression_From_Query(const AST *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    assert(q->edge_count != 0);

    AlgebraicExpression *exp = _AE_MUL(q->edge_count + q->node_count);
    int transpose; // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;
    
    // Scan MATCH clause from left to right.
    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        AST_GraphEntity *match_element;
        Vector_Get(matchPattern, i, &match_element);

        if(match_element->t != N_LINK) continue;
        AST_LinkEntity *astEdge = (AST_LinkEntity*)match_element;
        transpose = (astEdge->direction == N_RIGHT_TO_LEFT);
        e = QueryGraph_GetEdgeByAlias(q, astEdge->ge.alias);
        assert(e);

        // TODO: we might want to delay matrix retrieval even further.
        GrB_Matrix mat = Edge_GetMatrix(e);
        dest = e->dest;
        src = e->src;

        if(transpose) {
            dest = e->src;
            src = e->dest;
        }

        if(exp->operand_count == 0) {
            exp->src_node = src;
            if(src->label) {
                GrB_Matrix srcMat = Node_GetMatrix(src);
                AlgebraicExpression_AppendTerm(exp, srcMat, false, false);
            }
        }

        // ()-[:A|:B.]->()
        // Create matrix M, where M = A+B+...
        int labelCount = AST_LinkEntity_LabelCount(astEdge);
        if(labelCount > 1) {
            GraphContext *gc = GraphContext_GetFromTLS();
            Graph *g = gc->g;

            GrB_Matrix m;
            GrB_Matrix_new(&m, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

            for(int i = 0; i < labelCount; i++) {
                char *label = astEdge->labels[i];
                Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
                if(!s) continue;
                GrB_Matrix l = Graph_GetRelationMatrix(g, s->id);
                GrB_Info info = GrB_eWiseAdd_Matrix_Semiring(m, NULL, NULL, Rg_structured_bool, m, l, NULL);                
            }
            mat = m;
        }

        unsigned int hops = 1;
        /* Expand fixed variable length edge */
        if(astEdge->length && AST_LinkEntity_FixedLengthEdge(astEdge)) {
            hops = astEdge->length->minHops;
        }

        for(int i = 0; i < hops; i++) {
            bool freeMatrix = (labelCount > 1);
            AlgebraicExpression_AppendTerm(exp, mat, transpose, freeMatrix);
        }

        if(dest->label) {
            GrB_Matrix destMat = Node_GetMatrix(dest);
            AlgebraicExpression_AppendTerm(exp, destMat, false, false);
        }
    }

    exp->dest_node = dest;
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, matchPattern, q, exp_count);
    expressions = _AlgebraicExpression_IsolateVariableLenExps(expressions, exp_count);
    // TODO memory leak (fails on [a|b] relations?)
    // AlgebraicExpression_Free(exp);
    free(exp->operands);
    free(exp);
    
    return expressions;
}

/* Evaluates an algebraic expression,
 * evaluation is done right to left due to matrix CSC representation 
 * the right most operand in the expression is a tiny extremely sparse matrix
 * this allows us to avoid computing multiplications of large matrices.
 * In the case an operand is marked for transpose, we will perform
 * the transpose once and update the expression. */
void AlgebraicExpression_Execute(AlgebraicExpression *ae, GrB_Matrix res) {
    assert(ae && res);
    size_t operand_count = ae->operand_count;
    assert(operand_count > 1);

    AlgebraicExpressionOperand leftTerm;
    AlgebraicExpressionOperand rightTerm;
    // Operate on a clone of the expression.
    AlgebraicExpressionOperand operands[operand_count];
    memcpy(operands, ae->operands, sizeof(AlgebraicExpressionOperand) * operand_count);

    /* Multiply left to right
     * A*B*C*D
     * X = A*B
     * Y = X*C
     * Z = Y*D */
    for (int i = 1; i < operand_count; i++)
    {
        // Multiply and reduce.
        leftTerm = operands[i-1];
        rightTerm = operands[i];
        

        /* Incase we're required to transpose right hand side operand 
         * perform transpose once and update original expression. */
        if (rightTerm.transpose)
        {
            GrB_Matrix t = rightTerm.operand;
            /* Graph matrices are immutable, create a new matrix. 
             * and transpose. */
            if (!rightTerm.free)
            {
                GrB_Index cols;
                GrB_Matrix_ncols(&cols, rightTerm.operand);
                GrB_Matrix_new(&t, GrB_BOOL, cols, cols);
            }
            GrB_transpose(t, GrB_NULL, GrB_NULL, rightTerm.operand, GrB_NULL);

            // Update local and original expressions.
            rightTerm.free = true;
            rightTerm.operand = t;
            rightTerm.transpose = false;
            ae->operands[i].free = rightTerm.free;
            ae->operands[i].operand = rightTerm.operand;
            ae->operands[i].transpose = rightTerm.transpose;
        }
        _AlgebraicExpression_Execute_MUL(res, leftTerm.operand, rightTerm.operand, GrB_NULL);

        // Quick return if C is ZERO, there's no way to make progress.
        GrB_Index nvals = 0;
        GrB_Matrix_nvals(&nvals, res);
        if(nvals == 0) break;

        // Assign result and update operands count.
        operands[i].operand = res;
    }
}

void AlgebraicExpression_RemoveTerm(AlgebraicExpression *ae, int idx, AlgebraicExpressionOperand *operand) {
    assert(idx >= 0 && idx < ae->operand_count);
    if(operand) *operand = ae->operands[idx];

    // Shift left.
    for(int i = idx; i < ae->operand_count-1; i++) {
        ae->operands[i] = ae->operands[i+1];
    }

    ae->operand_count--;
}

void AlgebraicExpression_Free(AlgebraicExpression* ae) {
    for(int i = 0; i < ae->operand_count; i++) {
        if(ae->operands[i].free) {
            GrB_Matrix_free(&ae->operands[i].operand);
        }
    }

    free(ae->operands);
    free(ae);
}

void AlgebraicExpression_Transpose(AlgebraicExpression *ae) {
    /* Actually transpose expression:
     * E = A*B*C 
     * Transpose(E) = 
     * = Transpose(A*B*C) = 
     * = CT*BT*AT */
    
    // Switch expression src and dest nodes.
    Node *n = ae->src_node;
    ae->src_node = ae->dest_node;
    ae->dest_node = n;

    _AlgebraicExpression_ReverseOperandOrder(ae);

    for(int i = 0; i < ae->operand_count; i++)
        ae->operands[i].transpose = !ae->operands[i].transpose;
}

AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperationNode(AL_EXP_OP op) {
    AlgebraicExpressionNode *node = rm_calloc(1, sizeof(AlgebraicExpressionNode));
    node->type = AL_OPERATION;
    node->operation.op = op;
    node->operation.reusable = false;
    node->operation.v = NULL;
    node->operation.l = NULL;
    node->operation.r = NULL;
    return node;
}

AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperandNode(GrB_Matrix operand) {
    AlgebraicExpressionNode *node = rm_calloc(1, sizeof(AlgebraicExpressionNode));
    node->type = AL_OPERAND;
    node->operand = operand;
    return node;
}

void AlgebraicExpressionNode_AppendLeftChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child) {
    assert(root && root->type == AL_OPERATION && root->operation.l == NULL);
    root->operation.l = child;
}

void AlgebraicExpressionNode_AppendRightChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child) {
    assert(root && root->type == AL_OPERATION && root->operation.r == NULL);
    root->operation.r = child;
}

// restructure tree
//              (*)
//      (*)               (+)
// (a)       (b)    (e0)       (e1)

// To
//               (+)
//       (*)                (*)  
// (ab)       (e0)    (ab)       (e1)

// Whenever we encounter a multiplication operation
// where one child is an addition operation and the other child
// is a multiplication operation, we'll replace root multiplication
// operation with an addition operation with two multiplication operations
// one for each child of the original addition operation, as can be seen above.
// we'll want to reuse the left handside of the multiplication.
void AlgebraicExpression_SumOfMul(AlgebraicExpressionNode **root) {
    if((*root)->type == AL_OPERATION && (*root)->operation.op == AL_EXP_MUL) {
        AlgebraicExpressionNode *l = (*root)->operation.l;
        AlgebraicExpressionNode *r = (*root)->operation.r;

        if((l->type == AL_OPERATION && l->operation.op == AL_EXP_ADD &&
            !(r->type == AL_OPERATION && r->operation.op == AL_EXP_ADD)) ||
            (r->type == AL_OPERATION && r->operation.op == AL_EXP_ADD &&
            !(l->type == AL_OPERATION && l->operation.op == AL_EXP_ADD))) {
            
            AlgebraicExpressionNode *add = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);
            AlgebraicExpressionNode *lMul = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
            AlgebraicExpressionNode *rMul = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);

            AlgebraicExpressionNode_AppendLeftChild(add, lMul);
            AlgebraicExpressionNode_AppendRightChild(add, rMul);
            
            if(l->operation.op == AL_EXP_ADD) {
                // Lefthand side is addition, righthand side is multiplication.
                AlgebraicExpressionNode_AppendLeftChild(lMul, r);
                AlgebraicExpressionNode_AppendRightChild(lMul, l->operation.l);
                AlgebraicExpressionNode_AppendLeftChild(rMul, r);
                AlgebraicExpressionNode_AppendRightChild(rMul, l->operation.r);

                // Mark r as reusable.
                if(r->type == AL_OPERATION) r->operation.reusable = true;
            } else {
                // Righthand side is addition, lefthand side is multiplication.
                AlgebraicExpressionNode_AppendLeftChild(lMul, l);
                AlgebraicExpressionNode_AppendRightChild(lMul, r->operation.l);
                AlgebraicExpressionNode_AppendLeftChild(rMul, l);
                AlgebraicExpressionNode_AppendRightChild(rMul, r->operation.r);

                // Mark r as reusable.
                if(l->type == AL_OPERATION) l->operation.reusable = true;
            }

            // TODO: free old root.
            *root = add;
            AlgebraicExpression_SumOfMul(root);
        } else {
            AlgebraicExpression_SumOfMul(&l);
            AlgebraicExpression_SumOfMul(&r);
        }
    }
}

// Forward declaration.
static GrB_Matrix _AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res);

static GrB_Matrix _AlgebraicExpression_Eval_ADD(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Expression already evaluated.
    if(exp->operation.v != NULL) return exp->operation.v;

    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Matrix r = NULL;
    GrB_Matrix l = NULL;
    GrB_Matrix inter = NULL;
    GrB_Descriptor desc = NULL; // Descriptor used for transposing.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    // Determine if left or right expression needs to be transposed.
    if(leftHand && leftHand->type == AL_OPERATION && leftHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    }
    
    if(rightHand && rightHand->type == AL_OPERATION && rightHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
    }

    // Evaluate right expressions.
    r = _AlgebraicExpression_Eval(exp->operation.r, res);

    // Evaluate left expressions,
    // if lefthandside expression requires a matrix
    // to hold its intermidate value allocate one here.
    if(leftHand->type == AL_OPERATION) {
        GrB_Matrix_nrows(&nrows, r);
        GrB_Matrix_ncols(&ncols, r);
        GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
        l = _AlgebraicExpression_Eval(exp->operation.l, inter);
    } else {
        l = _AlgebraicExpression_Eval(exp->operation.l, NULL);
    }

    // Perform addition.
    assert(GrB_eWiseAdd_Matrix_Semiring(res, NULL, NULL, Rg_structured_bool, l, r, desc) == GrB_SUCCESS);
    if(inter) GrB_Matrix_free(&inter);

    // Store intermidate if expression is marked for reuse.
    // TODO: might want to use inter if available.
    if(exp->operation.reusable) {
        assert(exp->operation.v == NULL);
        GrB_Matrix_dup(&exp->operation.v, res);
    }

    if(desc) GrB_Descriptor_free(&desc);
    return res;
}

static GrB_Matrix _AlgebraicExpression_Eval_MUL(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Expression already evaluated.
    if(exp->operation.v != NULL) return exp->operation.v;

    GrB_Descriptor desc = NULL; // Descriptor used for transposing.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    // Determine if left or right expression needs to be transposed.
    if(leftHand && leftHand->type == AL_OPERATION && leftHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    }
    
    if(rightHand && rightHand->type == AL_OPERATION && rightHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
    }

    // Evaluate right left expressions.
    GrB_Matrix r = _AlgebraicExpression_Eval(exp->operation.r, res);
    GrB_Matrix l = _AlgebraicExpression_Eval(exp->operation.l, res);

    // Perform multiplication.
    assert(GrB_mxm(res, NULL, NULL, Rg_structured_bool, l, r, desc) == GrB_SUCCESS);

    // Store intermidate if expression is marked for reuse.
    if(exp->operation.reusable) {
        assert(exp->operation.v == NULL);
        GrB_Matrix_dup(&exp->operation.v, res);
    }

    if(desc) GrB_Descriptor_free(&desc);
    return res;
}

static GrB_Matrix _AlgebraicExpression_Eval_TRANSPOSE(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Transpose is an unary operation which gets delayed.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    assert( !(leftHand && rightHand) && (leftHand || rightHand) ); // Verify unary.
    if(leftHand) return _AlgebraicExpression_Eval(leftHand, res);
    else return _AlgebraicExpression_Eval(rightHand, res);
}

static GrB_Matrix _AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    if(exp == NULL) return NULL;
    if(exp->type == AL_OPERAND) return exp->operand;

    // Perform operation.
    switch(exp->operation.op) {
        case AL_EXP_MUL:
            return _AlgebraicExpression_Eval_MUL(exp, res);

        case AL_EXP_ADD:
            return _AlgebraicExpression_Eval_ADD(exp, res);

        case AL_EXP_TRANSPOSE:
            return _AlgebraicExpression_Eval_TRANSPOSE(exp, res);

        default:
            assert(false);
    }    
    return res;
}

void AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    _AlgebraicExpression_Eval(exp, res);
}

static void _AlgebraicExpressionNode_UniqueNodes(AlgebraicExpressionNode *root, AlgebraicExpressionNode ***uniqueNodes) {
    if(!root) return;

    // Have we seen this node before?
    int nodeCount = array_len(*uniqueNodes);
    for(int i = 0; i < nodeCount; i++) if(root == (*uniqueNodes)[i]) return;

    *uniqueNodes = array_append((*uniqueNodes), root);

    if(root->type != AL_OPERATION) return;

    _AlgebraicExpressionNode_UniqueNodes(root->operation.r, uniqueNodes);
    _AlgebraicExpressionNode_UniqueNodes(root->operation.l, uniqueNodes);
}

void AlgebraicExpressionNode_Free(AlgebraicExpressionNode *root) {
    if(!root) return;

    // Delay free for nodes which are referred from multiple points.
    AlgebraicExpressionNode **uniqueNodes = array_new(AlgebraicExpressionNode*, 1);
    _AlgebraicExpressionNode_UniqueNodes(root, &uniqueNodes);

    // Free unique nodes.
    AlgebraicExpressionNode *node;
    int nodesCount = array_len(uniqueNodes);
    for(int i = 0; i < nodesCount; i++) {
        node = array_pop(uniqueNodes);
        if(node->operation.v) GrB_Matrix_free(&node->operation.v);
        rm_free(node);
    }
    array_free(uniqueNodes);
}

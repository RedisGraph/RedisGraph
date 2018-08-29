/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "algebraic_expression.h"
#include <assert.h>

AlgebraicExpressionResult *_AlgebraicExpressionResult_New(GrB_Matrix M, AlgebraicExpression *ae) {
    AlgebraicExpressionResult *aer = malloc(sizeof(AlgebraicExpressionResult));

    // Are we required to transpose result?
    /* TODO: It might be cheaper to transpose using a 
     * descriptor when performing the last multiplication on the output. */
    aer->m = M;
    aer->src_node = ae->src_node;
    aer->dest_node = ae->dest_node;
    /* Free either when a multiplication was performed,
     * or a single operand was transposed */
    aer->_free_m = (ae->operand_count > 1 || ae->operands[0].transpose);
    return aer;
}

AlgebraicExpression *_AE_MUL(size_t operand_count) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = AL_EXP_MUL;
    ae->operand_cap = operand_count + 2;
    ae->operand_count = operand_count;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    ae->edge = NULL;
    return ae;
}

int _intermidate_node(const Node *n) {
    /* ->()<- 
     * <-()->
     * ->()->
     * <-()<- */
    return ((Vector_Size(n->incoming_edges) > 1) ||
            (Vector_Size(n->outgoing_edges) > 1) ||
            ((Vector_Size(n->incoming_edges) > 0) && (Vector_Size(n->outgoing_edges) > 0)));
}

int _referred_entity(char *alias, TrieMap *ref_entities) {
    return TRIEMAP_NOTFOUND != TrieMap_Find(ref_entities, alias, strlen(alias));
}

int _referred_node(const Node *n, const QueryGraph *q, TrieMap *ref_entities) {
    char *alias = QueryGraph_GetNodeAlias(q, n);
    return _referred_entity(alias, ref_entities);
}

/* For every referenced edge, add edge source and destination nodes
 * as referenced entities. */
void _referred_edge_ends(TrieMap *ref_entities, const QueryGraph *q) {
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
        char *srcAlias = QueryGraph_GetNodeAlias(q, e->src);
        Vector_Push(aliases, srcAlias);
        char *destAlias = QueryGraph_GetNodeAlias(q, e->dest);
        Vector_Push(aliases, destAlias);
    }

    // Add edges ends as referenced entities.
    for(int i = 0; i < Vector_Size(aliases); i++) {
        Vector_Get(aliases, i, &alias);
        TrieMap_Add(ref_entities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }

    Vector_Free(aliases);
}

/* Break down expression into sub expressions.
 * considering referenced intermidate nodes and edges. */
AlgebraicExpression **_AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * q->edge_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    int transpose;     // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Edge *e = NULL;

    TrieMap *ref_entities = NewTrieMap();
    ReturnClause_ReferredEntities(ast->returnNode, ref_entities);
    CreateClause_ReferredEntities(ast->createNode, ref_entities);
    WhereClause_ReferredEntities(ast->whereNode, ref_entities);
    _referred_edge_ends(ref_entities, q);

    AlgebraicExpression *iexp = _AE_MUL(q->edge_count);
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
        
        if(transpose) dest = e->src;
        else dest = e->dest;

        // If edge is referenced, set expression edge pointer.
        if(_referred_entity(edge->ge.alias, ref_entities))
            iexp->edge = QueryGraph_GetEdgeRef(q, e);

        iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];

        /* If intermidate node is referenced, create a new algebraic expression. */
        if(_intermidate_node(dest) && _referred_node(dest, q, ref_entities)) {
            // Finalize current expression.
            iexp->dest_node = QueryGraph_GetNodeRef(q, dest);
            assert(iexp->operand_count > 0);
            if(iexp->edge) {
                printf("Strange\n");
                printf("iexp->operand_count: %zu\n", iexp->operand_count);
                assert(iexp->operand_count == 1);
            }
            
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

/* Return index of matrix operand within algebraic expression
 * which has the least number of none zero values. */
int _AlgebraicExpression_MinMatPos(const AlgebraicExpression *ae) {
    GrB_Matrix M;       // Matrix with least number of NNZ.
    GrB_Index nvals;    // number of NNZ.
    int minMatPos = 0;  // Position of matrix within ae operands.
    GrB_Index minNvals = ULLONG_MAX; // minimum number of NNZ.

    // Search for minumum.
    for(int i = 0; i < ae->operand_count; i++) {
        M = ae->operands[i].operand;
        GrB_Matrix_nvals(&nvals, M);
        if(nvals < minNvals) {
            minNvals = nvals;
            minMatPos = i;
        }
    }
    
    return minMatPos;
}

static inline void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B, GrB_Descriptor desc) {
    GrB_mxm(
        C,                  // Output
        NULL,               // Mask
        NULL,               // Accumulator
        GxB_LOR_LAND_BOOL,  // Semiring
        A,                  // First matrix
        B,                  // Second matrix
        desc                // Descriptor        
    );
}

// Reverse order of operand within expression,
// A*B*C will become C*B*A. 
void _AlgebraicExpression_ReverseOperandOrder(AlgebraicExpression *exp) {
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

AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    assert(q->edge_count != 0);

    AlgebraicExpression *exp = _AE_MUL(q->edge_count);
    int transpose; // Indicate if matrix operand needs to be transposed.    
    int operandIdx = 0;
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;
    
    // Scan MATCH clause from left to right.
    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        AST_GraphEntity *match_element;
        Vector_Get(matchPattern, i, &match_element);

        if(match_element->t != N_LINK) continue;
        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        transpose = (edge->direction == N_RIGHT_TO_LEFT);
        e = QueryGraph_GetEdgeByAlias(q, edge->ge.alias);
        assert(e);

        dest = e->dest;
        src = e->src;

        if(transpose) {
            dest = e->src;
            src = e->dest;
        }

        if(operandIdx == 0) exp->src_node = QueryGraph_GetNodeRef(q, src);

        exp->operands[operandIdx].operand = e->mat;
        exp->operands[operandIdx].transpose = transpose;
        exp->operands[operandIdx].free = false;
        operandIdx++;
    }
    exp->dest_node = QueryGraph_GetNodeRef(q, dest);
    
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, matchPattern, q, exp_count);
    AlgebraicExpression_Free(exp);

    /* Because matrices are column ordered, when multiplying A*B
     * we need to reverse the order: B*A. */
    for(int i = 0; i < *exp_count; i++) _AlgebraicExpression_ReverseOperandOrder(expressions[i]);
    return expressions;
}

AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae) {
    assert(ae);

    size_t operand_count = ae->operand_count;
    AlgebraicExpressionOperand operands[operand_count];
    memcpy(operands, ae->operands, sizeof(AlgebraicExpressionOperand) * operand_count);

    GrB_Matrix A = operands[0].operand;
    GrB_Matrix C = A;

    if(operand_count == 1) {
        /* When transposing we must duplicate, as graph matrices
         * should be considered immutable. */
        if(operands[0].transpose) {
            GrB_Matrix transposed;
            GrB_Index nrows;
            GrB_Matrix_nrows(&nrows, A);
            GrB_Matrix_new(&transposed, GrB_BOOL, nrows, nrows);
            assert(GrB_transpose(transposed, NULL, NULL, A, NULL) == GrB_SUCCESS);
            C = transposed;
        }
    } else {
        C = NULL;
        GrB_Descriptor desc;
        GrB_Descriptor_new(&desc);
        AlgebraicExpressionOperand leftTerm;
        AlgebraicExpressionOperand rightTerm;

        /* Multiply right to left,
         * A*B*C*D,
         * X = C*D
         * Y = B*X
         * Z = A*Y */
        while(operand_count > 1) {
            rightTerm = operands[operand_count-1];
            leftTerm = operands[operand_count-2];
            
            if(!C) {
                GrB_Index nrows;
                GrB_Index ncols;
                GrB_Matrix_nrows(&nrows, leftTerm.operand);
                GrB_Matrix_ncols(&ncols, rightTerm.operand);
                GrB_Matrix_new(&C, GrB_BOOL, nrows, ncols);
            }

            // Multiply and reduce.
            if(leftTerm.transpose) GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
            if(rightTerm.transpose) GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);

            _AlgebraicExpression_Execute_MUL(C, leftTerm.operand, rightTerm.operand, desc);

            // Restore descriptor to default.
            GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);
            GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

            // Assign result and update operands count.
            operands[operand_count-2].operand = C;
            operands[operand_count-2].transpose = false;
            operand_count--;
        }

        GrB_Descriptor_free(&desc);
    }    

    // if(ae->_transpose) GrB_transpose(C, NULL, NULL, C, NULL);
    return _AlgebraicExpressionResult_New(C, ae);
}

void AlgebraicExpression_ClearOperands(AlgebraicExpression *ae) {
    for(int i = 0; i < ae->operand_count; i++) {
        if(ae->operands[i].free)
            GrB_Matrix_free(&ae->operands[i].operand);
    }

    ae->operand_count = 0;
}

void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp, bool freeOp) {
    assert(ae);    
    if(ae->operand_count+1 >= ae->operand_cap) {
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
    if(ae->operand_count >= ae->operand_cap) {
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

void AlgebraicExpression_RemoveTerm(AlgebraicExpression *ae, int idx, AlgebraicExpressionOperand *operand) {
    if(idx < 0 || idx >= ae->operand_count) return;
    if(operand) *operand = ae->operands[idx];

    // Shift left.
    for(int i = idx; i < ae->operand_count-1; i++) {
        ae->operands[i] = ae->operands[i+1];
    }

    ae->operand_count--;
}

void AlgebraicExpression_Transpose(AlgebraicExpression *ae) {
    /* Actually transpose expression:
     * E = A*B*C 
     * Transpose(E) = 
     * = Transpose(A*B*C) = 
     * = CT*BT*AT */
    
    // Switch expression src and dest nodes.
    Node **n = ae->src_node;
    ae->src_node = ae->dest_node;
    ae->dest_node = n;

    _AlgebraicExpression_ReverseOperandOrder(ae);

    for(int i = 0; i < ae->operand_count; i++)
        ae->operands[i].transpose = !ae->operands[i].transpose;
}

void AlgebraicExpressionResult_Free(AlgebraicExpressionResult *aer) {
    if(aer->_free_m) GrB_Matrix_free(&aer->m);
    free(aer);
}

void AlgebraicExpression_Free(AlgebraicExpression* ae) {
    for(int i = 0; i < ae->operand_count; i++)
        if(ae->operands[i].free)
            GrB_Matrix_free(&ae->operands[i].operand);

    free(ae->operands);
    free(ae);
}

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
    aer->_transpose = ae->_transpose;
    /* Free either when a multiplication was performed,
     * or a single operand was transposed */
    aer->_free_m = (ae->operand_count > 1 || ae->operands[0].transpose);
    return aer;
}

AlgebraicExpression *_AE_MUL(size_t operand_count) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = AL_EXP_MUL;
    ae->operand_count = operand_count;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * operand_count);
    ae->_transpose = false;
    return ae;
}

int _intermidate_node(const Node *n) {
    /* ->()<- 
     * <-()->
     * ->()-> */
    return ((Vector_Size(n->incoming_edges) > 1) ||
            (Vector_Size(n->outgoing_edges) > 1) ||
            ((Vector_Size(n->incoming_edges) > 0) && (Vector_Size(n->outgoing_edges) > 0)));
}

int _referred_node(const Node *n, const QueryGraph *q, TrieMap *return_nodes) {
    char *node_alias = QueryGraph_GetNodeAlias(q, n);
    return TRIEMAP_NOTFOUND != TrieMap_Find(return_nodes, node_alias, strlen(node_alias));
}

/* Break down expression into sub expressions.
 * considering referenced intermidate nodes. */
AlgebraicExpression **_AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * q->edge_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    int transpose;     // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Edge *e = NULL;

    TrieMap *ref_nodes = NewTrieMap();
    ReturnClause_ReferredNodes(ast->returnNode, ref_nodes);
    CreateClause_ReferredNodes(ast->createNode, ref_nodes);
    WhereClause_ReferredNodes(ast->whereNode, ref_nodes);

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

        iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];

        /* If intermidate node is referenced create a new algebraic expression. */
        if(_intermidate_node(dest) && _referred_node(dest, q, ref_nodes)) {
            // Finalize current expression.
            iexp->dest_node = QueryGraph_GetNodeRef(q, dest);
            assert(iexp->operand_count > 0);
            
            /* Create a new algebraic expression. */
            iexp = _AE_MUL(exp->operand_count - operandIdx);
            iexp->operand_count = 0;
            iexp->src_node = expressions[expIdx-1]->dest_node;
            iexp->dest_node = exp->dest_node;
            expressions[expIdx++] = iexp;
        }
    }
    
    *exp_count = expIdx;
    TrieMap_Free(ref_nodes, TrieMap_NOP_CB);
    return expressions;
}

/* Return index of matrix operand within algebraic expression
 * which has the least number of none zero values. */
int _AlgebraicExpression_MinMatPos(const AlgebraicExpression *ae) {
    int minMatPos = 0;  // Position of matrix within ae operands.
    GrB_Index nvals;    // number of NNZ.
    GrB_Index minNvals; // minimum number of NNZ.
    
    GrB_Matrix M = ae->operands[0].operand;
    GrB_Matrix_nvals(&minNvals, M);

    // Search for minumum.
    for(int i = 1; i < ae->operand_count; i++) {
        M = ae->operands[i].operand;
        GrB_Matrix_nvals(&nvals, M);
        if(nvals < minNvals) {
            minNvals = nvals;
            minMatPos = i;
        }
    }
    
    return minMatPos;
}

inline void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B, GrB_Descriptor desc) {
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
        operandIdx++;
    }
    exp->dest_node = QueryGraph_GetNodeRef(q, dest);
    
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, matchPattern, q, exp_count);
    AlgebraicExpression_Free(exp);
    return expressions;
}

AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae) {
    assert(ae);

    GrB_Matrix A = ae->operands[0].operand;
    GrB_Matrix C = A;

    if(ae->operand_count == 1) {
        /* AlgebraicExpression_Transpose guarantees that
         * either the expression is marked as transposed or
         * the single term, but never both. */
        assert(!(ae->_transpose && ae->operands[0].transpose));

        /* When transposing we must duplicate, as graph matrices
         * should be considered immutable. */
        if(ae->operands[0].transpose) {
            GrB_Matrix transposed;
            GrB_Index nrows;
            GrB_Matrix_nrows(&nrows, A);
            GrB_Matrix_new(&transposed, GrB_BOOL, nrows, nrows);
            assert(GrB_transpose(transposed, NULL, NULL, A, NULL) == GrB_SUCCESS);
            C = transposed;
        }
    } else {
        GrB_Descriptor desc;
        GrB_Descriptor_new(&desc);

        GrB_Index nrows;
        GrB_Matrix_nrows(&nrows, A);
        GrB_Matrix_new(&C, GrB_BOOL, nrows, nrows);

        AlgebraicExpressionOperand leftTerm;
        AlgebraicExpressionOperand rightTerm;

        while(ae->operand_count > 1) {
            // Pick matrix with min NNZ.
            int minMatPos = _AlgebraicExpression_MinMatPos(ae);
            int secondOperandPos = -1;
            // Pick second operand, either left or right of minMatPos.
            if(minMatPos == ae->operand_count-1) {
                // Last operand, multiply to the right.
                secondOperandPos = minMatPos-1;
                rightTerm = ae->operands[minMatPos];
                leftTerm = ae->operands[secondOperandPos];
            } else if(minMatPos == 0) {
                // First operand, multiply to the left.
                secondOperandPos = 1;
                leftTerm = ae->operands[0];
                rightTerm = ae->operands[secondOperandPos];
            } else {
                /* Intermidate operand, prefer second operand
                * to have the least NNZ between the two options. */
                GrB_Index leftNvals;
                GrB_Index rightNvals;
                GrB_Matrix_nvals(&leftNvals, ae->operands[minMatPos-1].operand);
                GrB_Matrix_nvals(&rightNvals, ae->operands[minMatPos+1].operand);
                if(leftNvals < rightNvals) {
                    // Multiply to the right.
                    secondOperandPos = minMatPos-1;
                    rightTerm = ae->operands[minMatPos];
                    leftTerm = ae->operands[secondOperandPos];
                } else {
                    // Multiply to the left.
                    secondOperandPos = minMatPos+1;
                    leftTerm = ae->operands[minMatPos];
                    rightTerm = ae->operands[secondOperandPos];
                }
            }

            // Terms must be consecutive.
            assert(abs(minMatPos - secondOperandPos) == 1);

            // Multiply and reduce.
            if(leftTerm.transpose) GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
            if(rightTerm.transpose) GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);

            _AlgebraicExpression_Execute_MUL(C, leftTerm.operand, rightTerm.operand, desc);

            // Restore descriptor to default.
            GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);
            GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

            // Assign result to minMatPos position and shrink operands array.
            ae->operands[minMatPos].operand = C;
            ae->operands[minMatPos].transpose = false;

            // Shift left.
            for(int i = secondOperandPos; i < ae->operand_count-1; i++)
                ae->operands[i] = ae->operands[i+1];

            ae->operand_count--;
        }

        GrB_Descriptor_free(&desc);
    }

    if(ae->_transpose) GrB_transpose(C, NULL, NULL, C, NULL);
    return _AlgebraicExpressionResult_New(C, ae);
}

void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transpose) {
    assert(ae);
    ae->operand_count++;
    ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_count);
    ae->operands[ae->operand_count-1].transpose = transpose;
    ae->operands[ae->operand_count-1].operand = m;
}

void AlgebraicExpression_PrependTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transpose) {
    assert(ae);
    ae->operand_count++;
    ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_count);
    // TODO: might be optimized with memcpy.
    // Shift operands to the right, making room at the begining.
    for(int i = ae->operand_count-1; i > 0 ; i--) {
        ae->operands[i] = ae->operands[i-1];
    }

    ae->operands[0].transpose = transpose;
    ae->operands[0].operand = m;
}

void AlgebraicExpression_Transpose(AlgebraicExpression *ae) {

    // Switch expression src and dest nodes.
    Node **n = ae->src_node;
    ae->src_node = ae->dest_node;
    ae->dest_node = n;

    if(ae->operand_count == 1) {
        // Expression transpose is determined by its only term.
        ae->_transpose = false;
        ae->operands[0].transpose = !ae->operands[0].transpose;
    } else {
        // Flip expression transpose flag.
        ae->_transpose = !ae->_transpose;
    }
}

void AlgebraicExpressionResult_Free(AlgebraicExpressionResult *aer) {
    if(aer->_transpose) GrB_transpose(aer->m, NULL, NULL, aer->m, NULL);
    if(aer->_free_m) GrB_Matrix_free(&aer->m);
    free(aer);
}

void AlgebraicExpression_Free(AlgebraicExpression* ae) {
    free(ae->operands);
    free(ae);
}
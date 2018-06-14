#include "algebraic_expression.h"
#include <assert.h>

void GxB_Matrix_print(GrB_Matrix m) {
    bool x = false;
    GrB_Index rows;
    GrB_Index cols;

    GrB_Matrix_nrows(&rows, m);
    GrB_Matrix_ncols(&cols, m);

    for(GrB_Index i = 0; i < rows; i++) {
        for(GrB_Index j = 0; j < cols; j++) {
            GrB_Matrix_extractElement_BOOL(&x, m, i, j);
            printf("%d ", x);
            x = false;
        }
        printf("\n");
    }
    printf("\n\n\n");
}

AlgebraicExpressionResult *_AlgebraicExpressionResult_New(GrB_Matrix M, AlgebraicExpression *ae) {
    AlgebraicExpressionResult *aer = malloc(sizeof(AlgebraicExpressionResult));
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
    ae->operand_count = operand_count;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * operand_count);
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
AlgebraicExpression **_AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST_Query *ast, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * q->edge_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    int transpose;     // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Edge *e = NULL;
    Vector *matchPattern = ast->matchNode->graphEntities;

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

AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, const QueryGraph *q, size_t *exp_count) {
    assert(q->edge_count != 0);

    AlgebraicExpression *exp = _AE_MUL(q->edge_count);
    int transpose; // Indicate if matrix operand needs to be transposed.    
    int operandIdx = 0;
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;
    Vector *matchPattern = ast->matchNode->graphEntities;
    
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
    
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, q, exp_count);
    AlgebraicExpression_Free(exp);
    return expressions;
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

/* Computes value of expression. 
 * Currently we're only dealing with expressions containing a single 
 * operation (multiply) and a number of operands. */
AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae) {
    assert(ae);
    
    GrB_Matrix A = ae->operands[0].operand;
    GrB_Matrix C = A;
    int operand_count = ae->operand_count;
    
    if(operand_count == 1) {
        if(ae->operands[0].transpose) {
            GrB_Matrix dup;
            /* TODO: replace assert with if, log, exit 1. */
            assert(GrB_Matrix_dup(&dup, A) == GrB_SUCCESS);
            assert(GrB_transpose(dup, NULL, NULL, dup, NULL) == GrB_SUCCESS);
            C = dup;
        }
    }

    if(operand_count > 1) {
        GrB_Descriptor desc;
        GrB_Descriptor_new(&desc);
        if(ae->operands[0].transpose) GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

        GrB_Index nrows;
        GrB_Matrix_nrows(&nrows, A);
        GrB_Matrix_new(&C, GrB_BOOL, nrows, nrows);
        
        for(int i = 1; i < operand_count; i++) {
            GrB_Matrix B = ae->operands[i].operand;
            if(ae->operands[i].transpose) GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);

            _AlgebraicExpression_Execute_MUL(C, A, B, desc);
            A = C;

            // Restore descriptor to default.
            GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);
            GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
        }

        GrB_Descriptor_free(&desc);
    }
        
    AlgebraicExpressionResult *res = _AlgebraicExpressionResult_New(C, ae);
    return res;
}

void AlgebraicExpression_Free(AlgebraicExpression* ae) {
    free(ae->operands);
    free(ae);
}

void AlgebraicExpressionResult_Free(AlgebraicExpressionResult *aer) {
    if(aer->_free_m) GrB_Matrix_free(&aer->m);
    free(aer);
}

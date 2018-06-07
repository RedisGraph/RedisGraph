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
    }
}

AlgebraicExpressionResult *_AlgebraicExpressionResult_New(GrB_Matrix M, AlgebraicExpression *ae) {
    AlgebraicExpressionResult *aer = malloc(sizeof(AlgebraicExpressionResult));
    aer->m = M;
    aer->src_node = ae->_src_node;
    aer->dest_node = ae->_dest_node;
    aer->_free_m = ae->operand_count > 1;
    return aer;
}

AlgebraicExpression *_AE_New(AL_EXP_OP op, size_t operand_count) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = op;
    ae->operand_count = operand_count;
    ae->operands = malloc(sizeof(GrB_Matrix) * operand_count);
    return ae;
}

AlgebraicExpression *_AE_ADD(size_t operand_count) {
    return _AE_New(AL_EXP_ADD, operand_count);
}

AlgebraicExpression *_AE_MUL(size_t operand_count) {
    return _AE_New(AL_EXP_MUL, operand_count);
}

AlgebraicExpression *_AE_AND(size_t operand_count) {
    return _AE_New(AL_EXP_AND, operand_count);
}

int _intermidate_node(const Node *n) {
    return ( (Vector_Size(n->incoming_edges) > 0) && (Vector_Size(n->outgoing_edges) > 0) );
}

int _referred_node(const Node *n, const QueryGraph *q, TrieMap *return_nodes) {
    char *node_alias = QueryGraph_GetNodeAlias(q, n);
    return TRIEMAP_NOTFOUND != TrieMap_Find(return_nodes, node_alias, strlen(node_alias));
}

/* Constructs a algebraic expressions from query,
 * for the time being we're only handling simple queries
 * of the form (A)-[B]->(C)-[D]->(E)...
 * Caller is responsible for freeing return value. */
AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, const QueryGraph *q, size_t *exp_count) {
    Node *n;
    Edge *e;
    *exp_count = 0;
    int matrix_idx = 0; /* Matrix position within an expression. */

    /* Set of nodes which must have their ID set. */
    TrieMap *ref_nodes = NewTrieMap();
    ReturnClause_ReferredNodes(ast->returnNode, ref_nodes);
    CreateClause_ReferredNodes(ast->createNode, ref_nodes);
    WhereClause_ReferredNodes(ast->whereNode, ref_nodes);

    /* At most we're expecting N algebraic expressions, where N is the number of edges 
     * within the query graph. */
    AlgebraicExpression **expressions = calloc(q->edge_count, sizeof(AlgebraicExpression *));

    /* Start scanning from node without incoming edges. */
    Vector *start_nodes = QueryGraph_GetNDegreeNodes(q, 0);
    /* No nodes with income degree of 0, nothing to do. */
    if(Vector_Size(start_nodes) == 0) goto cleanup;
    Vector_Get(start_nodes, 0, &n);

    /* First algebraic expression. */
    AlgebraicExpression *ae = _AE_MUL(q->edge_count);
    expressions[(*exp_count)++] = ae;
    ae->_src_node = QueryGraph_GetNodeRef(q, n);

    /* Traverse node. */
    /* TODO: Make sure there's no cycle in the graph, otherwise we'll be stuck in this loop. */
    while(Vector_Size(n->outgoing_edges)) {
        /* Wrongly assuming relation is between only two types.
        *     if(n->label)
        *         _Algebraic_Expression_Apply_Filter(expression, n->label);
        */
        Vector_Get(n->outgoing_edges, 0, &e);

        /* If intermidate node is referenced by return clause
         * create a new algebraic expression. */
        if(_intermidate_node(n) && _referred_node(n, q, ref_nodes)) {
            /* Update previous algebraic expression. */
            ae->operand_count = matrix_idx;

            /* Create a new algebraic expression. */
            ae = _AE_MUL(q->edge_count);
            ae->_src_node = QueryGraph_GetNodeRef(q, n);
            expressions[(*exp_count)++] = ae;
            matrix_idx = 0; /* Reset matrix position within new expression. */
        }

        ae->operands[matrix_idx++] = e->mat;
        n = e->dest; /* Advance. */
        ae->_dest_node = QueryGraph_GetNodeRef(q, n);
    }
    ae->operand_count = matrix_idx; /* Update last algebraic expression operand count. */

cleanup:
    TrieMap_Free(ref_nodes, TrieMap_NOP_CB);
    Vector_Free(start_nodes);

    return expressions;
}

void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B) {
    GrB_mxm(
        C,                  // Output
        NULL,               // Mask
        NULL,               // Accumulator
        GxB_LOR_LAND_BOOL,  // Semiring
        A,
        B,
        NULL                // Descriptor        
    );
}

GrB_Matrix _AlgebraicExpression_Execute_BinaryOP(GrB_Matrix A, GrB_Matrix B, GrB_BinaryOp op) {
    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, A);
    
    GrB_Matrix C;
    GrB_Matrix_new(&C, GrB_BOOL, nrows, nrows);
    
    GrB_eWiseAdd_Matrix_BinaryOp(
        C,      // Output
        NULL,   // Mask
        NULL,   // Accumulator
        op,     // Binary op
        A,
        B,
        NULL    // Descriptor
    );

    return C;
}

GrB_Matrix _AlgebraicExpression_Execute_ADD(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B) {
    return _AlgebraicExpression_Execute_BinaryOP(A, B, GxB_LOR_BOOL);
}

GrB_Matrix _AlgebraicExpression_Execute_AND(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B) {
    return _AlgebraicExpression_Execute_BinaryOP(A, B, GxB_LAND_BOOL);
}

void _AlgebraicExpression_Execute_Op(AL_EXP_OP op, GrB_Matrix C, GrB_Matrix A, GrB_Matrix B) {
    switch(op) {
        case AL_EXP_ADD:
            _AlgebraicExpression_Execute_ADD(C, A, B);
            break;
        case AL_EXP_MUL:            
            _AlgebraicExpression_Execute_MUL(C, A, B);
            break;
        case AL_EXP_AND:
            break;
        default:
            assert(false);
            break;
    }
}

/* Computes value of expression. 
 * Currently we're only dealing with expressions containing a single 
 * operation (multiply) and a number of operands. */
AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae) {
    assert(ae);

    GrB_Matrix A = ae->operands[0];
    GrB_Matrix C = A;
    int operand_count = ae->operand_count;

    if(operand_count > 1) {
        GrB_Index nrows;
        GrB_Matrix_nrows(&nrows, A);
        GrB_Matrix_new(&C, GrB_BOOL, nrows, nrows);
        
        for(int i = 1; i < operand_count; i++) {
            GrB_Matrix B = ae->operands[i];            
            _AlgebraicExpression_Execute_Op(ae->op, C, A, B);
            A = C;
        }
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

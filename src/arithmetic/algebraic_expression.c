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

// TODO This function incorrectly returns true when a node is repeatedly referenced,
// as with (a)-[]->(a)
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

int _referred_node(const Node *n, TrieMap *ref_entities) {
    return _referred_entity(n->alias, ref_entities);
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
void _referred_variable_length_edges(TrieMap *ref_entities, Vector *matchPattern, const QueryGraph *q) {
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
AlgebraicExpression** _AlgebraicExpression_IsolateVariableLenExps(AlgebraicExpression **expressions, size_t *expCount) {
    /* Return value is a new set of expressions, where each variable length expression
      * is guaranteed to have a single operand, as such in the worst case the number of
      * expressions doubles + 1. */
    AlgebraicExpression **res = malloc(sizeof(AlgebraicExpression*) * ((*expCount) * 2) + 1);
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
            if(expIdx < *expCount-1 && ! expressions[expIdx+1]->edgeLength) {
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
AlgebraicExpression **_AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
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
        // TODO This will falsely trigger in the given cases, but that does not seem
        // to impact the results of these particular queries.
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

AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
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

        if(exp->operand_count == 0) {
            exp->src_node = src;
            if(src->mat) AlgebraicExpression_AppendTerm(exp, src->mat, false, false);
        }

        unsigned int hops = 1;
        /* Expand fixed variable length edge */
        if(edge->length && AST_LinkEntity_FixedLengthEdge(edge)) {
            hops = edge->length->minHops;
        }

        for(int i = 0; i < hops; i++) {
            AlgebraicExpression_AppendTerm(exp, e->mat, transpose, false);
        }

        if(dest->mat) AlgebraicExpression_AppendTerm(exp, dest->mat, false, false);
    }

    exp->dest_node = dest;
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, matchPattern, q, exp_count);
    expressions = _AlgebraicExpression_IsolateVariableLenExps(expressions, exp_count);
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

            // Quick return if C is ZERO, there's no way to make progress.
            GrB_Index nvals = 0;
            GrB_Matrix_nvals(&nvals, C);
            if(nvals == 0) break;

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

    return _AlgebraicExpressionResult_New(C, ae);
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

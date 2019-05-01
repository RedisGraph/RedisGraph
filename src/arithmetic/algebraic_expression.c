/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "algebraic_expression.h"
#include "arithmetic_expression.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include <assert.h>

static inline bool intermediate_node(uint node_idx, uint path_len) {
    return (node_idx > 0) && (node_idx < path_len - 1);
}

static inline bool _ExpressionIsVariableLength(const AlgebraicExpression *exp) {
    return exp->minHops != exp->maxHops;
}

static int* _setup_traversed_relations(const cypher_astnode_t *ast_relation) {
    GraphContext *gc = GraphContext_GetFromTLS();

    uint relationIDsCount = cypher_ast_rel_pattern_nreltypes(ast_relation);

    int *relationIDs;
    if(relationIDsCount > 0) {
        relationIDs = array_new(int, relationIDsCount);
        for(int i = 0; i < relationIDsCount; i++) {
            // TODO documentation error in libcypher-parser
            const cypher_astnode_t *ast_reltype = cypher_ast_rel_pattern_get_reltype(ast_relation, i);
            const char *reltype = cypher_ast_reltype_get_name(ast_reltype);
            Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
            if(!s) continue;
            relationIDs = array_append(relationIDs, s->id);
        }
    } else {
        relationIDs = array_new(int, 1);
        relationIDs = array_append(relationIDs, GRAPH_NO_RELATION);
    }

    return relationIDs;
}

AlgebraicExpression *_AE_MUL(size_t operand_cap) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = AL_EXP_MUL;
    ae->operand_cap = operand_cap;
    ae->operand_count = 0;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    ae->edge = NULL;
    ae->relation_ids = NULL;
    ae->minHops = 1;
    ae->maxHops = 1;
    return ae;
}

AlgebraicExpression* _NewAlgebraicExpression(uint operand_count, Node *src, Node *dest, Edge *e, uint src_idx, uint dest_idx, uint edge_idx) {
    AlgebraicExpression *exp = _AE_MUL(operand_count);
    exp->operand_count = 0;
    exp->src_node = src;
    exp->dest_node = dest;
    exp->edge = e;
    exp->src_node_idx = src_idx;
    exp->dest_node_idx = dest_idx;
    exp->edge_idx = edge_idx;

    return exp;
}
AlgebraicExpression* _CloneAlgebraicExpression(const AlgebraicExpression *orig) {
    AlgebraicExpression *clone = malloc(sizeof(AlgebraicExpression));
    clone->op = orig->op;
    clone->src_node = orig->src_node;
    clone->dest_node = orig->dest_node;
    clone->edge = orig->edge;
    clone->src_node_idx = orig->src_node_idx;
    clone->dest_node_idx = orig->dest_node_idx;
    clone->edge_idx = orig->edge_idx;
    clone->minHops = orig->minHops;
    clone->maxHops = orig->maxHops;
    clone->relation_ids = orig->relation_ids;

    // The clone does not contain the original's operands
    clone->operand_count = 0;
    clone->operand_cap = 4;
    clone->operands = malloc(sizeof(AlgebraicExpressionOperand) * clone->operand_cap);

    return clone;
}

/* Variable length expression must contain only a single operand, the edge being
 * traversed multiple times, in cases such as (:labelA)-[e*]->(:labelB) both label A and B
 * are applied via a label matrix operand, this function migrates A and B from a
 * variable length expression to other expressions. */
AlgebraicExpression** _AlgebraicExpression_IsolateVariableLenExps(AlgebraicExpression **expressions, size_t *expCount) {
    /* Return value is a new set of expressions, where each variable length expression
      * is guaranteed to have a single operand, as such in the worst case the number of
      * expressions doubles + 1. */
    AlgebraicExpression **res = malloc(sizeof(AlgebraicExpression*) * (*expCount * 2 + 1));
    size_t newExpCount = 0;

    AlgebraicExpression *exp = expressions[0];
    /* If the first expression has a variable length and a labeled source,
     * separate the source matrix into its own expression. */
    if (_ExpressionIsVariableLength(exp) && exp->src_node->mat) {
        // Remove src node matrix from expression.
        AlgebraicExpressionOperand op;
        AlgebraicExpression_RemoveTerm(exp, 0, &op);

        /* Create a new expression. */
        AlgebraicExpression *newExp = _NewAlgebraicExpression(1, exp->src_node, NULL, NULL, exp->src_node_idx, NOT_IN_RECORD, NOT_IN_RECORD);
        newExp->op = AL_EXP_UNARY; // Mark that this expression should be replaced by a scan or reduce op
        AlgebraicExpression_PrependTerm(newExp, op.operand, op.transpose, op.free);
        res[newExpCount++] = newExp;
    }

    /* Scan through each expression, separating expressions
     * that have a variable length edge. */
    for(size_t expIdx = 0; expIdx < *expCount; expIdx++) {
        AlgebraicExpression *exp = expressions[expIdx];
        if(_ExpressionIsVariableLength(exp) == false) {
            // Add fixed-length expressions without modification
            res[newExpCount++] = exp;
            continue;
        }

        // Add the variable-length expression
        res[newExpCount++] = exp;

        // If the expression has a labeled destination, separate it into its own expression.
        if(exp->dest_node->mat) {
            // Remove dest node matrix from expression.
            AlgebraicExpressionOperand op;
            AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, &op);

            /* See if dest mat can be prepended to the following expression.
             * If not create a new expression. */
            if(expIdx < *expCount-1 && (_ExpressionIsVariableLength(exp) == false)) {
                AlgebraicExpression_PrependTerm(expressions[expIdx+1], op.operand, op.transpose, op.free);
            } else {
                // The destination is being placed in the source field
                // AlgebraicExpression *newExp = _NewAlgebraicExpression(1, NULL, exp->dest_node, NULL, NOT_IN_RECORD, exp->dest_node_idx, NOT_IN_RECORD);
                AlgebraicExpression *newExp = _NewAlgebraicExpression(1, exp->dest_node, NULL, NULL, exp->dest_node_idx, NOT_IN_RECORD, NOT_IN_RECORD);
                newExp->op = AL_EXP_UNARY;
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
 * considering referenced intermediate nodes and edges. */
AlgebraicExpression **_AlgebraicExpression_Intermediate_Expressions(const AST *ast, AlgebraicExpression *exp, const cypher_astnode_t *path, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * exp->operand_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    bool transpose;     // Indicate if matrix operand needs to be transposed.
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;
    uint dest_node_idx;

    AlgebraicExpression *iexp = _AE_MUL(exp->operand_count);
    iexp->src_node = exp->src_node;
    iexp->dest_node = exp->dest_node;
    iexp->src_node_idx = exp->src_node_idx;
    iexp->dest_node_idx = exp->dest_node_idx;
    iexp->operand_count = 0;
    expressions[expIdx++] = iexp;

    uint nelems = cypher_ast_pattern_path_nelements(path);
    for(uint i = 1; i < nelems; i += 2) {
        const cypher_astnode_t *ast_rel = cypher_ast_pattern_path_get_element(path, i);
        cypher_astnode_type_t type = cypher_astnode_type(ast_rel);

        e = QueryGraph_GetEntityByASTRef(q, ast_rel);
        transpose = (cypher_ast_rel_pattern_get_direction(ast_rel) == CYPHER_REL_INBOUND);

        AR_ExpNode *expr = AST_GetEntity(ast, ast_rel);
        /* If edge is referenced, set expression edge pointer. */
        if (expr->record_idx != NOT_IN_RECORD && expr->operand.variadic.entity_alias) { // TODO what is actually necessary?
            iexp->edge = e;
            iexp->relation_ids = _setup_traversed_relations(ast_rel);
            if (expr->record_idx == NOT_IN_RECORD) {
                expr->record_idx = AST_AddAnonymousRecordEntry((AST*)ast);
            }
            iexp->edge_idx = expr->record_idx;
        } else {
            iexp->edge_idx = NOT_IN_RECORD;
        }

        /* If this is a variable length edge, which is not fixed in length
         * remember edge length. */

        // TODO delete if possible
        unsigned int hops = 1;
        const cypher_astnode_t *varlength = cypher_ast_rel_pattern_get_varlength(ast_rel);
        if (varlength) {
            const cypher_astnode_t *start_node = cypher_ast_range_get_start(varlength);
            const cypher_astnode_t *end_node = cypher_ast_range_get_end(varlength);
            unsigned int start = (start_node == NULL) ? 1 : AST_ParseIntegerNode(start_node);
             // TODO make better unbounded identifier
            unsigned int end = (end_node == NULL) ? 100 : AST_ParseIntegerNode(end_node);
            iexp->minHops = start;
            iexp->maxHops = end;
            iexp->edge = e;
            if (iexp->relation_ids == NULL) iexp->relation_ids = _setup_traversed_relations(ast_rel);
            /* Expand fixed variable length edge */
            if (end == start) hops = start;
        }

        if (transpose) {
            // TODO seemingly no need to modify record IDs
            dest = e->src;
            src = e->dest;
        } else {
            dest = e->dest;
            src = e->src;
        }

        if(operandIdx == 0 && src->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        for(int j = 0; j < hops; j++) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        if(dest->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        // Don't build intermediate expression for non-intermediate nodes (not first or last)
        if (intermediate_node(i + 1, nelems) == false) continue;

        dest_node_idx = AST_GetEntityRecordIdx(ast, cypher_ast_pattern_path_get_element(path, i + 1));
        // Don't build intermediate expression if destination node is not referenced
        if (dest_node_idx == NOT_IN_RECORD) continue;

        // Finalize current expression.
        iexp->dest_node = dest;
        iexp->dest_node_idx = dest_node_idx;


        /* Create a new algebraic expression. */
        AlgebraicExpression *prev_exp = expressions[expIdx-1];
        iexp = _CloneAlgebraicExpression(exp); // TODO delete this function
        // New expression's source is the previous expression's destination
        iexp->src_node = prev_exp->dest_node;
        iexp->src_node_idx = prev_exp->dest_node_idx;
        expressions[expIdx++] = iexp;
    }

    *exp_count = expIdx;
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

AlgebraicExpression **AlgebraicExpression_FromPath(const AST *ast, const QueryGraph *q, const cypher_astnode_t *path, size_t *exp_count) {
    uint edge_count = array_len(q->edges);
    uint node_count = array_len(q->nodes);
    assert(edge_count != 0);

    AlgebraicExpression *exp = _AE_MUL(edge_count + node_count);
    bool transpose; // Indicate if matrix operand needs to be transposed.
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;

    uint nelems = cypher_ast_pattern_path_nelements(path);

    exp->src_node_idx = AST_GetEntityRecordIdx(ast, cypher_ast_pattern_path_get_element(path, 0));
    exp->dest_node_idx = AST_GetEntityRecordIdx(ast, cypher_ast_pattern_path_get_element(path, nelems - 1));

    const cypher_astnode_t *ast_rel = NULL;
    // Scan relations in MATCH clause from left to right.
    for(uint i = 1; i < nelems; i += 2) {
        ast_rel = cypher_ast_pattern_path_get_element(path, i);

        e = QueryGraph_GetEntityByASTRef(q, ast_rel);

        // TODO: we might want to delay matrix retrieval even further.
        GrB_Matrix mat = Edge_GetMatrix(e);
        dest = e->dest;
        src = e->src;

        transpose = (cypher_ast_rel_pattern_get_direction(ast_rel) == CYPHER_REL_INBOUND);
        if(transpose) {
            dest = e->src;
            src = e->dest;
            // TODO I don't think record IDs need to be set
        }

        if(exp->operand_count == 0) {
            exp->src_node = src;
            // TODO I don't think src_node_idx needs to be set
            if(src->label) {
                GrB_Matrix srcMat = Node_GetMatrix(src);
                AlgebraicExpression_AppendTerm(exp, srcMat, false, false);
            }
        }

        bool freeMatrix = false;

        // ()-[:A|:B.]->()
        // Create matrix M, where M = A+B+...
        unsigned int labelCount = cypher_ast_rel_pattern_nreltypes(ast_rel);
        if(labelCount > 1) {
            // Only need to free matrix if we're building it here
            freeMatrix = true;
            GraphContext *gc = GraphContext_GetFromTLS();
            Graph *g = gc->g;

            GrB_Matrix m;
            GrB_Matrix_new(&m, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

            for(unsigned int i = 0; i < labelCount; i++) {
                const cypher_astnode_t *reltype = cypher_ast_rel_pattern_get_reltype(ast_rel, i);
                assert(reltype);
                const char *reltype_str = cypher_ast_reltype_get_name(reltype);
                Schema *s = GraphContext_GetSchema(gc, reltype_str, SCHEMA_EDGE);
                if(!s) continue;
                GrB_Matrix l = Graph_GetRelationMatrix(g, s->id);
                GrB_Info info = GrB_eWiseAdd_Matrix_Semiring(m, NULL, NULL, Rg_structured_bool, m, l, NULL);
            }
            mat = m;
        }

        /* Expand variable length edge */
        const cypher_astnode_t *varlength = cypher_ast_rel_pattern_get_varlength(ast_rel);
        if (varlength) {
            const cypher_astnode_t *start_node = cypher_ast_range_get_start(varlength);
            const cypher_astnode_t *end_node = cypher_ast_range_get_end(varlength);
            exp->minHops = (start_node == NULL) ? 1 : AST_ParseIntegerNode(start_node);
             // TODO make better unbounded identifier
            exp->maxHops = (end_node == NULL) ? 100 : AST_ParseIntegerNode(end_node);
        }

        // Only append the matrix operand multiple times if the traversal is of
        // a fixed length - variable length traversals are handled separately.
        unsigned int hops = 1;
        if (_ExpressionIsVariableLength(exp) == false) hops = exp->minHops;

        for(unsigned int i = 0; i < hops; i++) {
            AlgebraicExpression_AppendTerm(exp, mat, transpose, freeMatrix);
        }

        if (exp->edge) exp->relation_ids = _setup_traversed_relations(ast_rel);

        if(dest->label) {
            GrB_Matrix destMat = Node_GetMatrix(dest);
            AlgebraicExpression_AppendTerm(exp, destMat, false, false);
        }
    }

    exp->dest_node = dest;
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermediate_Expressions(ast, exp, path, q, exp_count);
    expressions = _AlgebraicExpression_IsolateVariableLenExps(expressions, exp_count);
    // TODO memory leak (fails on [a|b] relations?)
    // AlgebraicExpression_Free(exp);
    free(exp->operands);
    free(exp);

    /* Because matrices are column ordered, when multiplying A*B
     * we need to reverse the order: B*A. */
    for(int i = 0; i < *exp_count; i++) _AlgebraicExpression_ReverseOperandOrder(expressions[i]);
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

    /* Multiply right to left
     * A*B*C*D
     * X = C*D
     * Y = B*X
     * Z = A*Y */
    while(operand_count > 1) {
        // Multiply and reduce.
        rightTerm = operands[operand_count-1];
        leftTerm = operands[operand_count-2];

        /* Incase we're required to transpose left hand side operand
         * perform transpose once and update original expression. */
        if(leftTerm.transpose) {
            GrB_Matrix t = leftTerm.operand;
            /* Graph matrices are immutable, create a new matrix.
             * and transpose. */
            if(!leftTerm.free) {
                GrB_Index cols;
                GrB_Matrix_ncols(&cols, leftTerm.operand);
                GrB_Matrix_new(&t, GrB_BOOL, cols, cols);
            }
            GrB_transpose(t, GrB_NULL, GrB_NULL, leftTerm.operand, GrB_NULL);

            // Update local and original expressions.
            leftTerm.free = true;
            leftTerm.operand = t;
            leftTerm.transpose = false;
            ae->operands[operand_count-2].free = leftTerm.free;
            ae->operands[operand_count-2].operand = leftTerm.operand;
            ae->operands[operand_count-2].transpose = leftTerm.transpose;
        }

        _AlgebraicExpression_Execute_MUL(res, leftTerm.operand, rightTerm.operand, GrB_NULL);

        // Quick return if C is ZERO, there's no way to make progress.
        GrB_Index nvals = 0;
        GrB_Matrix_nvals(&nvals, res);
        if(nvals == 0) break;

        // Assign result and update operands count.
        operands[operand_count-2].operand = res;
        operand_count--;
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
    // TODO correct?
    uint tmp_idx = ae->src_node_idx;
    ae->src_node_idx = ae->dest_node_idx;
    ae->dest_node_idx = tmp_idx;

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

void AlgebraicExpression_ExtendRecord(AlgebraicExpression *ae) {
    AST *ast = AST_GetFromTLS();
    if (ae->src_node_idx == NOT_IN_RECORD) {
        // Anonymous node - make space for it in the Record
        ae->src_node_idx = AST_AddAnonymousRecordEntry(ast);
    }

    if (ae->dest_node_idx == NOT_IN_RECORD) {
        ae->dest_node_idx = AST_AddAnonymousRecordEntry(ast);
    }

    if (ae->edge && ae->edge_idx == NOT_IN_RECORD) {
        ae->edge_idx = AST_AddAnonymousRecordEntry(ast);
    }
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

/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../algebraic_expression.h"
#include "utils.h"
#include "../arithmetic_expression.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../algorithms/algorithms.h"

#include <assert.h>

static AlgebraicExpression *_AlgebraicExpression_OperationRemoveRightmostChild
(
	AlgebraicExpression *root  // Root from which to remove a child.
) {
	assert(root);
	if(root->type != AL_OPERATION) return NULL;

	// No child nodes to remove.
	if(AlgebraicExpression_ChildCount(root) == 0) return NULL;

	// Remove rightmost child.
	AlgebraicExpression *child = array_pop(root->operation.children);
	return child;
}

static AlgebraicExpression *_AlgebraicExpression_OperationRemoveLeftmostChild
(
	AlgebraicExpression *root   // Root from which to remove a child.
) {
	assert(root);
	if(root->type != AL_OPERATION) return NULL;

	uint child_count = AlgebraicExpression_ChildCount(root);
	// No child nodes to remove.
	if(child_count == 0) return NULL;

	// Remove leftmost child.
	AlgebraicExpression *child = root->operation.children[0];

	// Shift left by 1.
	for(uint i = 0; i < child_count - 1; i++) {
		root->operation.children[i] = root->operation.children[i + 1];
	}
	array_pop(root->operation.children);

	return child;
}

static bool _AlgebraicExpression_IsMultiplicationNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_MUL);
}

static bool _AlgebraicExpression_IsAdditionNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_ADD);
}

// Locate the left most node in `exp`
static AlgebraicExpression *_leftMostNode(AlgebraicExpression *exp) {
	AlgebraicExpression *left_most = exp;
	while(left_most->type == AL_OPERATION && AlgebraicExpression_ChildCount(left_most) > 0) {
		left_most = FIRST_CHILD(left_most);
	}
	return left_most;
}

// Locate the right most node in `exp`
static AlgebraicExpression *_rightMostNode(AlgebraicExpression *exp) {
	AlgebraicExpression *right_most = exp;
	while(right_most->type == AL_OPERATION && AlgebraicExpression_ChildCount(right_most) > 0) {
		right_most = LAST_CHILD(right_most);
	}
	return right_most;
}

static bool __AlgebraicExpression_MulOverSum(AlgebraicExpression **root) {
	if(_AlgebraicExpression_IsMultiplicationNode(*root)) {
		AlgebraicExpression *l = (*root)->operation.children[0];
		AlgebraicExpression *r = (*root)->operation.children[1];

		// Do not care for (A + B) * (C + D)
		// As this will end up performing A*C + A*D + B*C + B*D
		// which is 4 multiplications and 3 additions compared to the original
		// 2 additions and one multiplication.
		if((_AlgebraicExpression_IsAdditionNode(l) && !_AlgebraicExpression_IsAdditionNode(r)) ||
		   (_AlgebraicExpression_IsAdditionNode(r) && !_AlgebraicExpression_IsAdditionNode(l))) {

			AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
			AlgebraicExpression *lMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
			AlgebraicExpression *rMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);

			AlgebraicExpression_AddChild(add, lMul);
			AlgebraicExpression_AddChild(add, rMul);

			AlgebraicExpression *A;
			AlgebraicExpression *B;
			AlgebraicExpression *C;

			if(l->operation.op == AL_EXP_ADD) {
				// Lefthand side is addition.
				// (A + B) * C = (A * C) + (B * C)

				A = _AlgebraicExpression_OperationRemoveLeftmostChild(l);
				B = _AlgebraicExpression_OperationRemoveRightmostChild(l);
				C = r;

				AlgebraicExpression_Free(l);
				AlgebraicExpression_AddChild(lMul, A);
				AlgebraicExpression_AddChild(lMul, C);
				AlgebraicExpression_AddChild(rMul, B);
				AlgebraicExpression_AddChild(rMul, AlgebraicExpression_Clone(C));

				// TODO: Mark r as reusable.
				// if(r->type == AL_OPERATION) r->operation.reusable = true;
			} else {
				// Righthand side is addition.
				// C * (A + B) = (C * A) + (C * B)

				A = _AlgebraicExpression_OperationRemoveLeftmostChild(r);
				B = _AlgebraicExpression_OperationRemoveRightmostChild(r);
				C = l;

				AlgebraicExpression_Free(r);
				AlgebraicExpression_AddChild(lMul, C);
				AlgebraicExpression_AddChild(lMul, A);
				AlgebraicExpression_AddChild(rMul, AlgebraicExpression_Clone(C));
				AlgebraicExpression_AddChild(rMul, B);

				// TODO: Mark r as reusable.
				// if(l->type == AL_OPERATION) l->operation.reusable = true;
			}
			*root = add;
			return true;
		}
	}

	// Recurse.
	uint child_count = AlgebraicExpression_ChildCount(*root);
	for(uint i = 0; i < child_count; i++) {
		if(__AlgebraicExpression_MulOverSum((*root)->operation.children + i)) return true;
	}
	return false;
}

// Distributive, multiplication over addition:
// A * (B + C) = (A * B) + (A * C)
//
//           (*)
//   (A)             (+)
//            (B)          (C)
//
// Becomes
//
//               (+)
//       (*)                (*)
// (A)        (B)     (A)        (C)
//
// Whenever we encounter a multiplication operation
// where one child is an addition operation and the other child
// is a multiplication operation, we'll replace root multiplication
// operation with an addition operation with two multiplication operations
// one for each child of the original addition operation, as can be seen above.
// we'll want to reuse the left handside of the multiplication.
static void _AlgebraicExpression_MulOverSum(AlgebraicExpression **root) {
	// As long as the tree changes keep modifying.
	while(__AlgebraicExpression_MulOverSum(root));
}

static void _AlgebraicExpression_CollectOperands(AlgebraicExpression *root, AlgebraicExpression ***operands) {
    uint child_count = 0;

    switch(root->type) {
        case AL_OPERAND:
        *operands = array_append(*operands, root);
        break;
        case AL_OPERATION:
        switch(root->operation.op) {
            case AL_EXP_TRANSPOSE:
            // Transposed is considered as an operand.
            *operands = array_append(*operands, root);
            break;
            case AL_EXP_ADD:
            case AL_EXP_MUL:
            child_count = AlgebraicExpression_ChildCount(root);
            for(uint i = 0; i < child_count; i++) {
                _AlgebraicExpression_CollectOperands(AlgebraicExpression_Clone(CHILD_AT(root, i)), operands);
            }
            default:
            assert("Unknow algebraic expression operation type" && false);
        }
        default:
        assert("Unknow algebraic expression node type" && false);
    }
}

static void _AlgebraicExpression_FlattenMultiplications(AlgebraicExpression *root) {
    assert(root);
    uint child_count;

    switch(root->type) {
        case AL_OPERATION:
        switch(root->operation.op) {
            case AL_EXP_ADD:
            case AL_EXP_TRANSPOSE:
                child_count = AlgebraicExpression_ChildCount(root);
                for(int i = 0; i < child_count; i++) _AlgebraicExpression_FlattenMultiplications(CHILD_AT(root, i));
            break;
            
            case AL_EXP_MUL:
            // Root has sub multiplication node(s).
            if(AlgebraicExpression_OperationCount(root, AL_EXP_MUL) > 1) {
                uint child_count = AlgebraicExpression_ChildCount(root);
                AlgebraicExpression **flat_children = array_new(AlgebraicExpression *, child_count);
                _AlgebraicExpression_CollectOperands(root, &flat_children);
                for(uint i = 0; i < child_count; i++) AlgebraicExpression_Free(CHILD_AT(root, i));
                array_free(root->operation.children);
                root->operation.children = flat_children;
            }

            break;
            default:
                assert("Unknown algebraic operation type" && false);
            break;
        }
        default:
        break;
    }
}

/* Multiplies `exp` to the left by `lhs`.
 * Returns new expression root.
 * `lhs` = (A + B)
 * `exp` = Transpose(C)
 * Returns (A + B) * Transpose(C) where `*` is the new root. */
static AlgebraicExpression *_AlgebraicExpression_MultiplyToTheLeft
(
	AlgebraicExpression *lhs,
	AlgebraicExpression *exp
) {
	assert(lhs && exp);
	AlgebraicExpression *mul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
	AlgebraicExpression_AddChild(mul, lhs);
	AlgebraicExpression_AddChild(mul, exp);
	return mul;
}

/* Multiplies `exp` to the right by `rhs`.
 * Returns new expression root.
 * `exp` = Transpose(C)
 * `rhs` = (A + B)
 * Returns Transpose(C) * (A + B) where `*` is the new root. */
static AlgebraicExpression *_AlgebraicExpression_MultiplyToTheRight
(
	AlgebraicExpression *exp,
	AlgebraicExpression *rhs
) {
	assert(exp && rhs);
	AlgebraicExpression *mul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
	AlgebraicExpression_AddChild(mul, exp);
	AlgebraicExpression_AddChild(mul, rhs);
	return mul;
}

/* Adds `exp` to the left by `lhs`.
 * Returns new expression root.
 * `lhs` = (A * B)
 * `exp` = Transpose(C)
 * Returns (A * B) + Transpose(C) where `+` is the new root. */
static AlgebraicExpression *_AlgebraicExpression_AddToTheLeft
(
	AlgebraicExpression *lhs,
	AlgebraicExpression *exp
) {
	assert(lhs && exp);
	AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
	AlgebraicExpression_AddChild(add, lhs);
	AlgebraicExpression_AddChild(add, exp);
	return add;
}

/* Adds `exp` to the right by `rhs`.
 * Returns new expression root.
 * `exp` = Transpose(C)
 * `rhs` = (A * B)
 * Returns Transpose(C) + (A * B) where `+` is the new root. */
static AlgebraicExpression *_AlgebraicExpression_AddToTheRight
(
	AlgebraicExpression *exp,
	AlgebraicExpression *rhs
) {
	assert(exp && rhs);
	AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
	AlgebraicExpression_AddChild(add, exp);
	AlgebraicExpression_AddChild(add, rhs);
	return add;
}

//------------------------------------------------------------------------------
// Static internal functions.
//------------------------------------------------------------------------------

/* Node with (income + outcome degree) > 2
 * is considered a highly connected node. */
static bool _highly_connected_node(const QueryGraph *qg, const char *alias) {
	// Look up node in qg.
	QGNode *n = QueryGraph_GetNodeByAlias(qg, alias);
	return ((array_len(n->incoming_edges) + array_len(n->outgoing_edges)) > 2);
}

static inline bool _referred_entity(const char *alias) {
	AST *ast = QueryCtx_GetAST();
	return AST_AliasIsReferenced(ast, alias);
}

/* If the edge is referenced or of a variable length, it should populate the AlgebraicExpression. */
static inline bool _should_populate_edge(QGEdge *e) {
	return (_referred_entity(e->alias) || QGEdge_VariableLength(e));
}

/* Checks if given expression contains a variable length edge. */
static bool _AlgebraicExpression_ContainsVariableLengthEdge
(
	const QueryGraph *qg,
	const AlgebraicExpression *root
) {
	uint child_count = 0;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			if(_AlgebraicExpression_ContainsVariableLengthEdge(qg, root->operation.children[i])) return true;
		}
		return false;
	case AL_OPERAND:
		if(root->operand.edge) {
			QGEdge *e = QueryGraph_GetEdgeByAlias(qg, root->operand.edge);
			return QGEdge_VariableLength(e);
		}
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}
	return false;
}

static void _RemovePathFromGraph(QueryGraph *g, QGEdge **path) {
	uint edge_count = array_len(path);
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *e = path[i];
		QGNode *src = e->src;
		QGNode *dest = e->dest;

		QueryGraph_RemoveEdge(g, e);
		QGEdge_Free(e);
		if(QGNode_EdgeCount(src) == 0) {
			QueryGraph_RemoveNode(g, src);
			QGNode_Free(src);
		}
		if(src != dest && QGNode_EdgeCount(dest) == 0) {
			QueryGraph_RemoveNode(g, dest);
			QGNode_Free(dest);
		}
	}
}

/* Variable length expression must contain only a single operand, the edge being
 * traversed multiple times, in cases such as (:labelA)-[e*]->(:labelB) both label A and B
 * are applied via a label matrix operand, this function migrates A and B from a
 * variable length expression to other expressions. */
static AlgebraicExpression **_AlgebraicExpression_IsolateVariableLenExps(
	const QueryGraph *qg, AlgebraicExpression **expressions) {
	/* Return value is a new set of expressions, where each variable length expression
	 * is guaranteed to have a single operand, as such in the worst case the number of
	 * expressions doubles + 1. */
	size_t expCount = array_len(expressions);
	AlgebraicExpression **res = array_new(AlgebraicExpression *, expCount * 2 + 1);

	/* Scan through each expression, locate expression which
	 * have a variable length edge in them. */
	for(size_t expIdx = 0; expIdx < expCount; expIdx++) {
		AlgebraicExpression *exp = expressions[expIdx];
		if(!_AlgebraicExpression_ContainsVariableLengthEdge(qg, exp)) {
			res = array_append(res, exp);
			continue;
		}

		// Expression contains a variable length edge.
		QGNode *src = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Source(exp));
		QGNode *dest = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Destination(exp));

		// A variable length expression with a labeled source node
		// We only care about the source label matrix, when it comes to
		// the first expression, as in the following expressions
		// src is the destination of the previous expression.
		if(expIdx == 0 && src->label) {
			// Remove src node matrix from expression.
			AlgebraicExpression *op = AlgebraicExpression_RemoveLeftmostNode(exp);
			res = array_append(res, op);
		}

		res = array_append(res, exp);

		// If the expression has a labeled destination, separate it into its own expression.
		if(dest->label) {
			// Remove dest node matrix from expression.
			AlgebraicExpression *op = AlgebraicExpression_RemoveRightmostNode(exp);

			/* See if dest mat can be prepended to the following expression.
			 * If not create a new expression. */
			if(expIdx < expCount - 1 &&
			   !_AlgebraicExpression_ContainsVariableLengthEdge(qg, expressions[expIdx + 1])) {
				_AlgebraicExpression_MultiplyToTheLeft(expressions[expIdx + 1], op);
			} else {
				res = array_append(res, op);
			}
		}
	}

	array_free(expressions);
	return res;
}

static inline void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B,
													GrB_Descriptor desc) {
	// A,B,C must be boolean matrices.
	GrB_Info res = GrB_mxm(
					   C,                   // Output
					   GrB_NULL,            // Mask
					   GrB_NULL,            // Accumulator
					   GxB_LOR_LAND_BOOL,   // Semiring
					   A,                   // First matrix
					   B,                   // Second matrix
					   desc                 // Descriptor
				   );

	if(res != GrB_SUCCESS) {
		// If the multiplication failed, print error info to stderr and exit.
		fprintf(stderr, "Enountered error in matrix multiplication:\n%s\n", GrB_error());
		assert(false);
	}
}

static inline bool _should_divide_expression(QGEdge **path, int idx, const QueryGraph *qg) {
	QGEdge *e = path[idx];

	return (_should_populate_edge(e)                    ||  // This edge is populated.
			_should_populate_edge(path[idx + 1])        ||  // The next edge is populated.
			_highly_connected_node(qg, e->dest->alias)  ||  // Destination node in+out degree > 2.
			_referred_entity(e->dest->alias));              // Destination node is referenced.
}

/* Break down path into sub paths.
 * Considering referenced intermidate nodes and edges. */
static QGEdge ***_Intermediate_Paths
(
	QGEdge **path,
	const QueryGraph *qg
) {
	QGEdge *e = NULL;
	int pathLen = array_len(path);

	/* Allocating maximum number of expression possible. */
	QGEdge ***paths = array_new(QGEdge **, pathLen);
	QGEdge **intermediate_path = array_new(QGEdge *, 1);

	/* Scan path left to right,
	 * construct intermidate paths by "breaking" on referenced entities. */
	for(int i = 0; i < pathLen - 1; i++) {
		e = path[i];
		intermediate_path = array_append(intermediate_path, e);
		if(_should_divide_expression(path, i, qg)) {
			// Break! add current path to paths and create a new path.
			paths = array_append(paths, intermediate_path);
			intermediate_path = array_new(QGEdge *, 1);
		}
	}

	// Handle last hop.
	e = path[pathLen - 1];
	intermediate_path = array_append(intermediate_path, e);
	// If `intermidate_path` contains just the last hop it's not part of paths, add it.
	if(array_len(intermediate_path) == 1) paths = array_append(paths, intermediate_path);

	return paths;
}

static AlgebraicExpression *_AlgebraicExpression_OperandFromNode
(
	QGNode *n
) {
	GrB_Matrix mat;
	bool free = false;
	bool diagonal = true;
	bool transpose = false;
	Graph *g = QueryCtx_GetGraph();

	if(n->labelID == GRAPH_UNKNOWN_LABEL) mat = Graph_GetZeroMatrix(g);
	else mat = Graph_GetLabelMatrix(g, n->labelID);

	return AlgebraicExpression_NewOperand(mat, free, diagonal, n->alias, n->alias, NULL);
}

static AlgebraicExpression *_AlgebraicExpression_OperandFromEdge
(
	QGEdge *e,
	bool transpose
) {
	GrB_Matrix mat;
	uint reltype_id;
	Graph *g = QueryCtx_GetGraph();
	AlgebraicExpression *add;
	AlgebraicExpression *root = NULL;

	uint reltype_count = array_len(e->reltypeIDs);
	switch(reltype_count) {
	case 0: // No relationship types specified; use the full adjacency matrix
		mat = Graph_GetAdjacencyMatrix(g);
		root = AlgebraicExpression_NewOperand(mat, false, false, e->src->alias, e->dest->alias,
											  e->alias);
		break;
	case 1: // One relationship type
		reltype_id = e->reltypeIDs[0];
		if(reltype_id == GRAPH_UNKNOWN_RELATION) mat = Graph_GetZeroMatrix(g);
		else mat = Graph_GetRelationMatrix(g, e->reltypeIDs[0]);
		root = AlgebraicExpression_NewOperand(mat, false, false, e->src->alias, e->dest->alias,
											  e->alias);
		break;
	default: // Multiple edge type: -[:A|:B]->
		add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
		for(uint i = 0; i < reltype_count; i++) {
			uint reltype_id = e->reltypeIDs[i];
			// No matrix to add
			if(reltype_id == GRAPH_UNKNOWN_RELATION) mat = Graph_GetZeroMatrix(g);
			else mat = Graph_GetRelationMatrix(g, reltype_id);
			AlgebraicExpression *operand = AlgebraicExpression_NewOperand(mat, false, false,
																		  e->src->alias, e->dest->alias, e->alias);
			AlgebraicExpression_AddChild(add, operand);
		}
		root = add;
		break;
	}

	/* Expand fixed variable length edge.
	 * -[A*2..2]->
	 * A*A
	 * -[A|B*2..2]->
	 * (A+B) * (A+B) */
	if(!QGEdge_VariableLength(e) && e->minHops > 1) {
		AlgebraicExpression *mul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
		AlgebraicExpression_AddChild(mul, root);
		for(int i = 1; i < e->minHops; i++) {
			// Clone to avoid double free.
			AlgebraicExpression_AddChild(mul, AlgebraicExpression_Clone(root));
		}
		root = mul;
	}

	if(transpose) {
		AlgebraicExpression *op_transpose = AlgebraicExpression_NewOperation(AL_EXP_TRANSPOSE);
		AlgebraicExpression_AddChild(op_transpose, root);
		root = op_transpose;
	}

	// If src node has a label, multiply to the left by label matrix.
	if(e->src->label) {
		root = _AlgebraicExpression_MultiplyToTheLeft(_AlgebraicExpression_OperandFromNode(e->src), root);
	}

	return root;
}

/* In case edges `a` and `b` share a node:
 * (a)-[E0]->(b)<-[E1]-(c)
 * than the shared entity is returned
 * if edges are disjoint, NULL is returned. */
static QGNode *_SharedNode
(
	const QGEdge *a,
	const QGEdge *b
) {
	assert(a && b);
	if(a->dest == b->src) return a->dest;   // (a)-[E0]->(b)-[E1]->(c)
	if(a->src == b->dest) return a->src;    // (a)<-[E0]-(b)<-[E1]-(c)
	if(a->src == b->src) return a->src;     // (a)<-[E0]-(b)-[E1]->(c)
	if(a->dest == b->dest) return a->dest;  // (a)-[E0]->(b)<-[E1]-(c)
	return NULL;
}

static void _reversePath
(
	QGEdge **path,
	uint path_len,
	bool *transpositions
) {
	for(uint i = 0; i < path_len; i++) {
		/* A reversed path should have its transpositions flipped
		 * transpose(transpose(A)) = A */
		transpositions[i] = !transpositions[i];
	}

	/* Transpose(A*B) = Transpose(B) * Transpose(A)
	 * (a)<-[A]-(b)<-[B]-(c)-[C]->(d)
	 * At * Bt * C
	 * Transpose(At * Bt * C) =
	 * = Ct * B * A
	 * (d)-[Ct]->(c)-[B]->(b)-[A]->(a) */

	// Reverse the path array as well as the transposition array
	for(uint i = 0; i < path_len / 2; i++) {
		uint opposite = path_len - i - 1;
		QGEdge *tmp = path[opposite];
		path[opposite] = path[i];
		path[i] = tmp;

		bool transpose_tmp = transpositions[opposite];
		transpositions[opposite] = transpositions[i];
		transpositions[i] = transpose_tmp;
	}
}

static void _normalizePath
(
	QGEdge **path,          // Path to normalize.
	uint path_len,          // Path length.
	bool *transpositions    // Specifies which edges need to be transposed.
) {
	// Initialize `transpositions` array.
	for(uint i = 0; i < path_len; i++) transpositions[i] = false;

	// A single leg path.
	if(path_len <= 1) return;

	uint transposeCount = 0;
	// For every edge except the last:
	for(uint i = 0; i < path_len - 1; i++) {
		QGEdge *e = path[i];
		QGEdge *follow = path[i + 1];
		QGNode *shared = _SharedNode(e, follow);
		assert(shared);

		/* The edge should be transposed if its destination is not shared.
		 * (dest)<-[e]-(shared)-[follow]->()
		 * (dest)<-[e]-(shared)<-[follow]-() */
		if(e->dest != shared) {
			transpositions[i] = true;
			transposeCount++;
		}
	}

	// For the last edge, transpose if its source is not shared.
	QGEdge *e = path[path_len - 1];
	QGNode *shared = _SharedNode(path[path_len - 2], e);
	if(e->src != shared) {
		transposeCount++;
		transpositions[path_len - 1] = true;
	}

	// Reverse entire path if the majority of edges must be transposed.
	if(transposeCount > (path_len - transposeCount)) {
		_reversePath(path, path_len, transpositions);
	}

	// Apply transpose.
	for(uint i = 0; i < path_len; i++) {
		QGEdge *e = path[i];
		if(transpositions[i]) QGEdge_Reverse(e);
	}
}

static AlgebraicExpression *_AlgebraicExpression_FromPath
(
	QGEdge **path
) {
	assert(path);

	QGEdge *e = NULL;
	uint path_len = array_len(path);
	assert(path_len > 0);
	AlgebraicExpression *root = NULL;

	/* Treating path as a chain
	 * we're aligning all edges to "point right"
	 * (A)-[E0]->(B)-[E0]->(C)-[E0]->(D).
	 * e.g.
	 * (A)-[E0]->(B)<-[E1]-(C)-[E2]->(D)
	 * E1 will be transposed:
	 * (A)-[E0]->(B)-[E1]->(C)-[E2]->(D) */

	bool transpositions[path_len];
	_normalizePath(path, path_len, transpositions);

	// Construct expression.
	for(int i = 0; i < path_len; i++) {
		e = path[i];
		// Add Edge matrix.
		AlgebraicExpression *op = _AlgebraicExpression_OperandFromEdge(e, transpositions[i]);

		if(!root) {
			root = op;
		} else {
			// Connect via a multiplication node.
			root = _AlgebraicExpression_MultiplyToTheRight(root, op);
		}
	}   // End of path traversal.

	// If last node on path has a label, multiply by label matrix.
	if(e->dest->label) {
		root = _AlgebraicExpression_MultiplyToTheRight(root, _AlgebraicExpression_OperandFromNode(e->dest));
	}

	return root;
}

static AlgebraicExpression *_AlgebraicExpression_CloneOperation
(
	const AlgebraicExpression *exp
) {
	AlgebraicExpression *clone = AlgebraicExpression_NewOperation(exp->operation.op);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) {
		AlgebraicExpression_AddChild(clone, AlgebraicExpression_Clone(exp->operation.children[i]));
	}
	return clone;
}

static AlgebraicExpression *_AlgebraicExpression_CloneOperand
(
	const AlgebraicExpression *exp
) {
	return AlgebraicExpression_NewOperand(exp->operand.matrix, exp->operand.free,
										  exp->operand.diagonal, exp->operand.src,
										  exp->operand.dest, exp->operand.edge);
}

//------------------------------------------------------------------------------
// AlgebraicExpression construction.
//------------------------------------------------------------------------------

// Construct algebraic expression form query graph.
AlgebraicExpression **AlgebraicExpression_FromQueryGraph
(
	const QueryGraph *qg,   // Query-graph to process
	uint *exp_count         // Number of algebraic expressions generated.
) {
	assert(qg && exp_count);

	/* Construct algebraic expression(s) from query-graph.
	 * Trying to take advantage of long multiplications with as few
	 * transpose as possible we'll transform paths crossing the graph
	 * "diameter", these are guarantee to be the longest, although
	 * there might be situations in which these are not the most optimal paths
	 * to explore.
	 *
	 * Once a path been transformed it's removed from the query-graph and the process
	 * repeat itself. */

	/* A graph with no edges implies an empty algebraic expression
	 * the reasoning behind this decission is that algebraic expression
	 * represent graph traversals, no edges means no traversals. */
	uint edge_count = QueryGraph_EdgeCount(qg);
	if(edge_count == 0) {
		*exp_count = 0;
		return NULL;
	}

	bool acyclic = IsAcyclicGraph(qg);
	QueryGraph *g = QueryGraph_Clone(qg);
	AlgebraicExpression **exps = array_new(AlgebraicExpression *, 1);

	// As long as the query-graph isn't empty.
	while(QueryGraph_EdgeCount(g) > 0) {
		// Get leaf nodes at the deepest level.
		int depth;
		QGNode *n;
		if(acyclic) n = LongestPathTree(g, &depth); // Graph is a tree.
		else n = LongestPathGraph(g, &depth);       // Graph contains cycles.

		// Get a path of length level, allow closing a cycle if the graph is not acyclic.
		QGEdge **path = DFS(n, depth, !acyclic);
		uint path_len = array_len(path);
		assert(path_len == depth);

		/* TODO:
		 * In case path is a cycle, e.g. (b)-[]->(a)-[]->(b)
		 * make sure the first node on the path is referenced, _should_divide_expression(path, 0) is true.
		 * if this is not the case we will unnecessarily break the generated expression into 2 sub expressions
		 * while what we can do is simply rotate the cycle, (a)-[]->(b)-[]->(a)
		 * this is exactly the same only now we won't sub divide.
		 * Checking if path is a cycle done by testing the start and end node. */

		// Split path into sub paths.
		QGEdge ***paths = _Intermediate_Paths(path, qg);
		AlgebraicExpression **sub_exps = array_new(AlgebraicExpression *, 1);

		uint path_count = array_len(paths);
		for(uint i = 0; i < path_count; i++) {
			// Construct expression.
			AlgebraicExpression *exp = _AlgebraicExpression_FromPath(paths[i]);
			sub_exps = array_append(sub_exps, exp);

			/* Remove exp[i] src label matrix (left most operand) as it's
			 * being used by exp[i-1] dest label matrix.
			 * (:A)-[:X]->(:B)-[:Y]->(:C)
			 * exp0: A * X * B
			 * exp1: B * Y * C
			 * should become
			 * exp0: A * X * B
			 * exp1: Y * C */
			if(i > 0) {
				QGNode *src = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Source(exp));
				if(src->label) {
					/* exp[i] shares a label matrix with exp[i-1]
					 * remove redundancy. */
					AlgebraicExpression *redundent = AlgebraicExpression_RemoveLeftmostNode(exp);
					AlgebraicExpression_Free(redundent);
				}
			}
			// Expression can not be empty.
			assert(AlgebraicExpression_ChildCount(exp) > 0);
		}

		sub_exps = _AlgebraicExpression_IsolateVariableLenExps(qg, sub_exps);

		uint sub_count = array_len(sub_exps);
		printf("sub_count: %d\n", sub_count);
		for(uint i = 0; i < sub_count; i++) {
			AlgebraicExpression *exp = sub_exps[i];
			printf("src: %s\n", AlgebraicExpression_Source(exp));
			printf("dest: %s\n", AlgebraicExpression_Destination(exp));

			AlgebraicExpression_Print(exp);
			printf("\n");
			AlgebraicExpression_PrintTree(exp);
			printf("\n\n\n");

			_AlgebraicExpression_MulOverSum(&exp);

			AlgebraicExpression_Print(exp);
			printf("\n");
			AlgebraicExpression_PrintTree(exp);
			printf("\n\n\n");

			// Add constructed expression to return value.
			exps = array_append(exps, exp);
		}

		// Remove path from graph.
		_RemovePathFromGraph(g, path);

		// Clean up
		for(uint i = 0; i < path_count; i++) array_free(paths[i]);
		array_free(paths);
		array_free(sub_exps);

		/* If original graph contained a cycle
		 * see now after we've removed a path if this is still the case. */
		if(!acyclic) acyclic = IsAcyclicGraph(g);
	}

	// TODO just return exps?
	*exp_count = array_len(exps);
	AlgebraicExpression **res = rm_malloc(sizeof(AlgebraicExpression *) * (*exp_count));
	for(size_t i = 0; i < (*exp_count); i++) res[i] = exps[i];
	array_free(exps);

	QueryGraph_Free(g);

	return res;
}

//------------------------------------------------------------------------------
// AlgebraicExpression Node creation functions.
//------------------------------------------------------------------------------

// Create a new AlgebraicExpression operation node.
AlgebraicExpression *AlgebraicExpression_NewOperation
(
	AL_EXP_OP op    // Operation to perform.
) {
	AlgebraicExpression *node = rm_malloc(sizeof(AlgebraicExpression));
	node->type = AL_OPERATION;
	node->operation.op = op;
	node->operation.children = array_new(AlgebraicExpression *, 0);
	return node;
}

// Create a new AlgebraicExpression operand node.
AlgebraicExpression *AlgebraicExpression_NewOperand
(
	GrB_Matrix mat,     // Matrix.
	bool free,          // Should operand be free when we're done.
	bool diagonal,      // Is operand a diagonal matrix?
	const char *src,    // Operand row domain (src node).
	const char *dest,   // Operand column domain (destination node).
	const char *edge    // Operand alias (edge).
) {
	AlgebraicExpression *node = rm_malloc(sizeof(AlgebraicExpression));
	node->type = AL_OPERAND;
	node->operand.matrix = mat;
	node->operand.free = free;
	node->operand.diagonal = diagonal;
	node->operand.src = src;
	node->operand.dest = dest;
	node->operand.edge = edge;
	return node;
}

// Clone algebraic expression node.
AlgebraicExpression *AlgebraicExpression_Clone
(
	const AlgebraicExpression *exp  // Expression to clone.
) {
	assert(exp);
	switch(exp->type) {
	case AL_OPERATION:
		return _AlgebraicExpression_CloneOperation(exp);
		break;
	case AL_OPERAND:
		return _AlgebraicExpression_CloneOperand(exp);
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}
	return NULL;
}

//------------------------------------------------------------------------------
// AlgebraicExpression attributes.
//------------------------------------------------------------------------------

// Returns the source entity represented by the left-most operand row domain.
const char *AlgebraicExpression_Source
(
	AlgebraicExpression *root   // Root of expression.
) {
	assert(root);
	while(root->type == AL_OPERATION) {
		root = root->operation.children[0];
	}

	return root->operand.src;
}

// Returns the destination entity represented by the right-most operand column domain.
const char *AlgebraicExpression_Destination
(
	AlgebraicExpression *root   // Root of expression.
) {
	assert(root);
	while(root->type == AL_OPERATION) {
		uint child_count = AlgebraicExpression_ChildCount(root);
		root = root->operation.children[child_count - 1];
	}

	return root->operand.dest;
}

/* Returns the first edge alias encountered.
 * if no edge alias is found NULL is returned. */
const char *AlgebraicExpression_Edge
(
	const AlgebraicExpression *root   // Root of expression.
) {
	assert(root);

	uint child_count = 0;
	const char *edge = NULL;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			edge = AlgebraicExpression_Edge(CHILD_AT(root, i));
			if(edge) return edge;
		}
		break;
	case AL_OPERAND:
		return root->operand.edge;
	}

	return NULL;
}

// Returns the number of child nodes directly under root.
uint AlgebraicExpression_ChildCount
(
	const AlgebraicExpression *root   // Root of expression.
) {
	assert(root);
	if(root->type == AL_OPERATION) return array_len(root->operation.children);
	else return 0;
}

// Returns the number of operands in expression.
uint AlgebraicExpression_OperandCount
(
	const AlgebraicExpression *root
) {
	uint operand_count = 0;
	uint child_count = 0;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			operand_count += AlgebraicExpression_OperandCount(CHILD_AT(root, i));
		}
		break;
	case AL_OPERAND:
		operand_count = 1;
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}
	return operand_count;
}

// Returns the number of operations in expression.
uint AlgebraicExpression_OperationCount
(
	const AlgebraicExpression *root,
	AL_EXP_OP op_type
) {
	uint op_count = 0;
	if(root->type == AL_OPERATION) {
		if(root->type & op_type) op_count = 1;
		uint child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			op_count += AlgebraicExpression_OperationCount(CHILD_AT(root, i), op_type);
		}
	}
	return op_count;
}

//------------------------------------------------------------------------------
// AlgebraicExpression modification functions.
//------------------------------------------------------------------------------

// Adds child node to root children list.
void AlgebraicExpression_AddChild
(
	AlgebraicExpression *root,  // Root to attach child to.
	AlgebraicExpression *child  // Child node to attach.
) {
	assert(root && root->type == AL_OPERATION);
	root->operation.children = array_append(root->operation.children, child);
}

// Remove leftmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveLeftmostNode
(
	AlgebraicExpression *root   // Root from which to remove left most child.
) {
	assert(root);
	AlgebraicExpression *prev = root;
	AlgebraicExpression *current = root;

	while(current->type == AL_OPERATION) {
		prev = current;
		current = current->operation.children[0];
	}
	assert(current->type == AL_OPERAND);

	/* Removing an operand from an operation
	 * this might cause a replacement of the operation:
	 * MUL(A,B) after removing A will become just B
	 * TRANSPOSE(A) after removing A should become NULL. */
	if(prev->type == AL_OPERATION) {
		_AlgebraicExpression_OperationRemoveLeftmostChild(prev);
		uint child_count = AlgebraicExpression_ChildCount(prev);
		if(child_count < 2) {
			if(child_count == 1) {
				AlgebraicExpression *replacement = _AlgebraicExpression_OperationRemoveRightmostChild(prev);
				/* Free operation internals, doesn't free `prev` as it might be pointed
				 * at by other operations. */
				_AlgebraicExpression_FreeOperation(prev);
				// Overide prev with replacement.
				memcpy(prev, replacement, sizeof(AlgebraicExpression));
				// Free replacement as it has been copied.
				rm_free(replacement);
			} else {
				assert("for the timebing, we should not be here" && false);
			}
		}
	}
	return current;
}

// Remove rightmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveRightmostNode
(
	AlgebraicExpression *root   // Root from which to remove left most child.
) {
	assert(root);
	AlgebraicExpression *prev = root;
	AlgebraicExpression *current = root;

	while(current->type == AL_OPERATION) {
		prev = current;
		uint child_count = AlgebraicExpression_ChildCount(current);
		current = current->operation.children[child_count - 1];
	}
	assert(current->type == AL_OPERAND);

	/* Removing an operand from an operation
	 * this might cause a replacement of the operation:
	 * MUL(A,B) after removing A the expression will become just B.
	 * TRANSPOSE(A) after removing A should become NULL. */
	if(prev->type == AL_OPERATION) {
		_AlgebraicExpression_OperationRemoveRightmostChild(prev);
		uint child_count = AlgebraicExpression_ChildCount(prev);
		if(child_count < 2) {
			if(child_count == 1) {
				AlgebraicExpression *replacement = _AlgebraicExpression_OperationRemoveRightmostChild(prev);
				/* Free operation internals, doesn't free `prev` as it might be pointed
				 * at by other operations. */
				_AlgebraicExpression_FreeOperation(prev);
				// Overide prev with replacement.
				memcpy(prev, replacement, sizeof(AlgebraicExpression));
				// Free replacement as it has been copied.
				rm_free(replacement);
			} else {
				printf("Warning, operation with no child operands, e.g. empty transpose.\n");
			}
		}
	}
	return current;
}

/* Multiply root to the left with op.
 * Updates root. */
void AlgebraicExpression_MultiplyToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *rhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current left most operand. */
	AlgebraicExpression *left_most_operand = _leftMostNode(rhs);
	AlgebraicExpression *lhs = AlgebraicExpression_NewOperand(m, false, false,
															  left_most_operand->operand.src, left_most_operand->operand.dest, left_most_operand->operand.edge);

	*root = _AlgebraicExpression_MultiplyToTheLeft(lhs, rhs);
}

/* Multiply root to the right with op.
 * Updates root. */
void AlgebraicExpression_MultiplyToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *lhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current right most operand. */
	AlgebraicExpression *right_most_operand = _rightMostNode(lhs);
	AlgebraicExpression *rhs = AlgebraicExpression_NewOperand(m, false, false,
															  right_most_operand->operand.src, right_most_operand->operand.dest,
															  right_most_operand->operand.edge);

	*root = _AlgebraicExpression_MultiplyToTheRight(lhs, rhs);
}

// Add expression to the left by operand
// A + (exp)
void AlgebraicExpression_AddToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *rhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current left most operand. */
	AlgebraicExpression *left_most_operand = _leftMostNode(rhs);
	AlgebraicExpression *lhs = AlgebraicExpression_NewOperand(m, false, false,
															  left_most_operand->operand.src, left_most_operand->operand.dest, left_most_operand->operand.edge);

	*root = _AlgebraicExpression_AddToTheLeft(lhs, rhs);
}

// Add expression to the right by operand
// (exp) + A
void AlgebraicExpression_AddToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *lhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current right most operand. */
	AlgebraicExpression *right_most_operand = _rightMostNode(lhs);
	AlgebraicExpression *rhs = AlgebraicExpression_NewOperand(m, false, false,
															  right_most_operand->operand.src, right_most_operand->operand.dest,
															  right_most_operand->operand.edge);

	*root = _AlgebraicExpression_AddToTheRight(lhs, rhs);
}

// Returns true if entire expression is transposed.
bool AlgebraicExpression_Transposed
(
	const AlgebraicExpression *root   // Root of expression.
) {
	return (root->type == AL_OPERATION && root->operation.op == AL_EXP_TRANSPOSE);
}

// Returns true if expression contains operation.
bool AlgebraicExpression_ContainsOp
(
	const AlgebraicExpression *root,
	AL_EXP_OP op
) {
	if(root->type == AL_OPERATION) {
		if(root->operation.op == op) return true;
		uint child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			if(AlgebraicExpression_ContainsOp(CHILD_AT(root, i), op)) return true;
		}
	}
	return false;
}

// Checks to see if operand at position `operand_idx` is a diagonal matrix.
bool AlgebraicExpression_DiagonalOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
) {
	const AlgebraicExpression *op = _AlgebraicExpression_GetOperand(root, operand_idx);
	assert(op && op->type == AL_OPERAND);
	return op->operand.diagonal;
}

//------------------------------------------------------------------------------
// AlgebraicExpression optimizations
//------------------------------------------------------------------------------
void AlgebraicExpression_Optimize
(
	AlgebraicExpression **exp
) {
	assert(exp);
	_AlgebraicExpression_MulOverSum(exp);
	_AlgebraicExpression_FlattenMultiplications(*exp);
}

//------------------------------------------------------------------------------
// AlgebraicExpression free
//------------------------------------------------------------------------------

// Free algebraic expression.
void AlgebraicExpression_Free
(
	AlgebraicExpression *root  // Root node.
) {
	assert(root);
	switch(root->type) {
	case AL_OPERATION:
		_AlgebraicExpression_FreeOperation(root);
		break;
	case AL_OPERAND:
		_AlgebraicExpression_FreeOperand(root);
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}
	rm_free(root);
}

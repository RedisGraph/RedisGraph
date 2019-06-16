/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_EXPAND_INTO_H
#define __OP_EXPAND_INTO_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../graph/graph.h"
#include "../../graph/entities/edge.h"
#include "../../arithmetic/algebraic_expression.h"

typedef struct {
    OpBase op;
    AST *ast;
    Graph *graph;
    AlgebraicExpression *ae;
    GrB_Matrix F;               // Filter matrix.
    GrB_Matrix M;               // Algebraic expression result.
    int *edgeRelationTypes;     // One or more relation types.
    int edgeRelationCount;      // length of edgeRelationTypes.
    Edge *edges;                // Discovered edges.
    GxB_MatrixTupleIter *iter;  // Iterator over M.
    uint srcNodeRecIdx;         // Index into record.
    uint destNodeRecIdx;        // Index into record.
    uint edgeRecIdx;            // Index into record.
    uint recordsCap;            // Max number of records to process.
    uint recordsLen;            // Number of records to process.
    bool transposed_edge;       // Track whether the expression references a transposed edge.
    Record *records;            // Array of records.
    Record r;                   // Current selected record.
} OpExpandInto;

OpBase* NewExpandIntoOp(AlgebraicExpression *algebraic_expression, AST *ast);
OpResult ExpandIntoInit(OpBase *opBase);
Record ExpandIntoConsume(OpBase *opBase);
OpResult ExpandIntoReset(OpBase *ctx);
void ExpandIntoFree(OpBase *ctx);

#endif

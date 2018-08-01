/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_index_scan.h"

OpBase *NewIndexScanOp(QueryGraph *qg, Graph *g, Node **node, IndexIter *iter) {
  IndexScan *indexScan = malloc(sizeof(IndexScan));
  indexScan->g = g;
  indexScan->node = node;
  indexScan->_node = *node;
  indexScan->iter = iter;

  // Set our Op operations
  OpBase_Init(&indexScan->op);
  indexScan->op.name = "Index Scan";
  indexScan->op.type = OPType_INDEX_SCAN;
  indexScan->op.consume = IndexScanConsume;
  indexScan->op.reset = IndexScanReset;
  indexScan->op.free = IndexScanFree;

  indexScan->op.modifies = NewVector(char*, 1);
  Vector_Push(indexScan->op.modifies, QueryGraph_GetNodeAlias(qg, *node));

  return (OpBase*)indexScan;
}

  /*
     TODO deletes probably make the basic assumption that IDs are immutable invalid
   */
OpResult IndexScanConsume(OpBase *opBase, QueryGraph* graph) {
  IndexScan *op = (IndexScan*)opBase;

  GrB_Index *node_id = IndexIter_Next(op->iter);
  if (!node_id) return OP_DEPLETED;

  *op->node = Graph_GetNode(op->g, *node_id);

  // TODO should be unnecessary
  if (*op->node == NULL) {
    return OP_DEPLETED;
  }

  (*op->node)->id = *node_id;

  return OP_OK;
}

OpResult IndexScanReset(OpBase *ctx) {
  IndexScan *indexScan = (IndexScan*)ctx;

  /* Restore original node. */
  *indexScan->node = indexScan->_node;
  // Verify that this call does what we want
  IndexIter_Reset(indexScan->iter);

  return OP_OK;
}

void IndexScanFree(OpBase *op) {
  IndexScan *indexScan = (IndexScan *)op;
  IndexIter_Free(indexScan->iter);
}


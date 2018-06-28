#include "op_index_scan.h"

OpBase *NewIndexScanOp(QueryGraph *qg, Graph *g, Node **node, IndexIterator *iter) {
  return (OpBase*)NewIndexScan(qg, g, node, iter);
}

IndexScan* NewIndexScan(QueryGraph *qg, Graph *g, Node **node, IndexIterator *iter) {

  IndexScan *indexScan = malloc(sizeof(IndexScan));
  /* TODO I think these are getting set to dummy nodes rather than proper
   * graph nodes. It is set before getting accessed by Consume, but an assertion
   * in the FilterTree _applyPredicateFilters fails if they are not set here.
   * Determine appropriate settings. */
  indexScan->node = node;
  indexScan->_node = *node;
  indexScan->iter = iter;
  indexScan->g = g;

  // Set our Op operations
  indexScan->op.name = "Index Scan";
  indexScan->op.type = OPType_INDEX_SCAN;
  indexScan->op.consume = IndexScanConsume;
  indexScan->op.reset = IndexScanReset;
  indexScan->op.free = IndexScanFree;
  indexScan->op.modifies = NewVector(char*, 1);

  Vector_Push(indexScan->op.modifies, QueryGraph_GetNodeAlias(g, *node));

  return indexScan;
}

OpResult IndexScanConsume(OpBase *opBase, QueryGraph* graph) {
  IndexScan *op = (IndexScan*)opBase;

  /* Update node */
  *op->node = IndexIterator_Next(op->iter);

  if(*op->node == NULL) {
    return OP_DEPLETED;
  }

  return OP_OK;
}

OpResult IndexScanReset(OpBase *ctx) {
  IndexScan *indexScan = (IndexScan*)ctx;

  /* Restore original node. */
  *indexScan->node = indexScan->_node;
  IndexIterator_Reset(indexScan->iter);

  return OP_OK;
}

void IndexScanFree(OpBase *op) {
  IndexScan *indexScan = (IndexScan *)op;
  IndexIterator_Free(indexScan->iter);
  free(indexScan);
}
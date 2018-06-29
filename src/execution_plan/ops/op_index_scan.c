#include "op_index_scan.h"

OpBase *NewIndexScanOp(QueryGraph *qg, Graph *g, Node **node, IndexCreateIter *iter) {
  return (OpBase*)NewIndexScan(qg, g, node, iter);
}

IndexScan* NewIndexScan(QueryGraph *qg, Graph *g, Node **node, IndexCreateIter *iter) {

  IndexScan *indexScan = malloc(sizeof(IndexScan));
  /* TODO I think these are getting set to dummy nodes rather than proper
   * graph nodes. It is set before getting accessed by Consume, but an assertion
   * in the FilterTree _applyPredicateFilters fails if they are not set here.
   * Determine appropriate settings. */
  indexScan->node = node;
  indexScan->_node = *node;
  indexScan->g = g;

  // Set our Op operations
  indexScan->op.name = "Index Scan";
  indexScan->op.type = OPType_INDEX_SCAN;
  indexScan->op.consume = IndexScanConsume;
  indexScan->op.reset = IndexScanReset;
  indexScan->op.free = IndexScanFree;
  indexScan->op.modifies = NewVector(char*, 1);

  Vector_Push(indexScan->op.modifies, QueryGraph_GetNodeAlias(qg, *node));

  // Make index matrix of size n*n
  int n = g->node_count;
  GrB_Matrix operand;
  GrB_Matrix_new(&operand, GrB_BOOL, n, n);

  GrB_Index node_id;
  while ((node_id = (GrB_Index)IndexCreateIter_Next(iter)) > 0) {
    // Obey constraint here?
    GrB_Matrix_setElement_BOOL(operand, true, node_id, node_id);
  }

  // Should be masked in IndexIterator
  indexScan->iter = TuplesIter_new(operand);

  return indexScan;
}

  /*
     TODO deletes probably make the basic assumption that IDs are immutable invalid
   */
OpResult IndexScanConsume(OpBase *opBase, QueryGraph* graph) {
  IndexScan *op = (IndexScan*)opBase;

  GrB_Index node_id;
  if (TuplesIter_next(op->iter, NULL, &node_id) == TuplesIter_DEPLETED) {
    return OP_DEPLETED;
  }

  *op->node = Graph_GetNode(op->g, node_id);
  // LabelScan sets id here
  if (*op->node == NULL) {
    return OP_DEPLETED;
  }

  return OP_OK;
}

OpResult IndexScanReset(OpBase *ctx) {
  IndexScan *indexScan = (IndexScan*)ctx;

  /* Restore original node. */
  *indexScan->node = indexScan->_node;
  // Verify that this call does what we want
  TuplesIter_reset(indexScan->iter);

  return OP_OK;
}

void IndexScanFree(OpBase *op) {
  IndexScan *indexScan = (IndexScan *)op;
  TuplesIter_free(indexScan->iter);
  free(indexScan);
}
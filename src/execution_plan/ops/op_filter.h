#ifndef _FILTER_H_
#define _FILTER_H_

#include "op.h"
#include "../../filter_tree/filter_tree.h"

/* FilterState 
 * Different states in which ExpandAll can be at. */
typedef enum {
    FilterUninitialized, /* Filter wasn't initialized it. */
    FilterRequestRefresh, /* */
    FilterResetted,      /* Filter was just restarted. */
} FilterState;

/* Filter
 * filters graph according to where cluase */
typedef struct {
    OpBase op;
    FT_FilterNode *filterTree;
    FilterState state;
} Filter;

/* Creates a new Filter operation */
OpBase* NewFilterOp(FT_FilterNode *filterTree);
Filter* NewFilter(FT_FilterNode *filterTree);

/* FilterConsume next operation 
 * returns OP_DEPLETED when */
OpResult FilterConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult FilterReset(OpBase *ctx);

/* Frees Filter*/
void FilterFree(OpBase *ctx);

#endif //_FILTER_H_
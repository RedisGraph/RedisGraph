#ifndef _NODE_ITERATOR_H
#define _NODE_ITERATOR_H

#include "node_block.h"

/* Node iterator iterates over a set of graph nodes */

typedef struct NodeIterator {
    NodeBlock *_start_block;    // Current block.
    NodeBlock *_current_block;
    int _start_pos;             // Iterator initial position.
    int _current_pos;           // Iterator current position.
    int _block_pos;             // Position within a block.
    int _end_pos;               // Iterator won't pass end position.
    int _step;                  // Increase current_pos by step each iteration.
} NodeIterator;

// Creates a new node iterator.
NodeIterator *NodeIterator_New (
    NodeBlock *block,       // Block from which iteration begins.
    int start_pos,          // Iteration starts here.
    int end_pos,            // Iteration stops here.
    int step                // To scan entire range, set step to 1.
);

// Clones given iterator.
NodeIterator *NodeIterator_Clone (
    const NodeIterator *it  // Iterator to clone.
);

// Returns the next node, unless we've reached the end
// in which case NULL is returned.
Node *NodeIterator_Next(NodeIterator *iter);

// Reset iterator to original position.
void NodeIterator_Reset(NodeIterator *iter);

// Free node iterator.
void NodeIterator_Free(NodeIterator *iter);

#endif
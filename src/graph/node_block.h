#ifndef _NODE_BLOCK_H
#define _NODE_BLOCK_H

#include "node.h"
#include <stdint.h>

/* Graph's nodes are hold within a linked list of blocks,
 * each block can hold a varying length of nodes,
 * at the beginning of each block you'll find the number of nodes
 * currently stored in the block, the block capacity and a pointer to
 * the next block if it exists. */

// Number of nodes in a block. Should always be a power of 2.
#define NODEBLOCK_CAP 256

typedef struct nodeBlock {
    size_t size;                    // Number of nodes in block.
    size_t cap;                     // Number of nodes block can hold.
    struct nodeBlock *next;         // Pointer to next block.
    Node nodes[];                   // Nodes array. MUST BE LAST MEMBER OF THE STRUCT!
} NodeBlock;

// Create a new NodeBlock with n nodes.
NodeBlock *NodeBlock_New();

// Free block.
void NodeBlock_Free(NodeBlock *block);

#endif
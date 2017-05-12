#ifndef NODE_H_
#define NODE_H_

#include "../rmutil/vector.h"

// Forward declaration of edge
struct Edge;
typedef struct Edge;

typedef struct {
	char* alias;			// an alias for this node
	char* id;				// node unique id (might be empty)
	int internalId;			// node unique id can not be empty
	Vector* outgoingEdges;	// list of incoming edges (ME)<-(SRC)
	Vector* incomingEdges;	// list on outgoing edges (ME)->(DEST)
} Node;

// Creates a new node.
Node* NewNode(const char* alias, const char* id);

// Creates a clone of given node.
Node* Node_Clone(const Node *node);

// Checks if nodes are "equal"
int Node_Compare(const Node *a, const Node *b);

int Node_IncomeDegree(const Node *n);

// Connects source node to destination node by edge
void ConnectNode(Node* src, Node* dest, struct Edge* e);

// Frees alocated space by given node.
void FreeNode(Node* node);

#endif
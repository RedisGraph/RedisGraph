#ifndef NODE_H_
#define NODE_H_

#include "graph_entity.h"
#include "../value.h"
#include "../rmutil/vector.h"

/* Forward declaration of edge */
struct Edge;
typedef struct {
	// GraphEntity entity;
	struct {
		long int id;
		int prop_count;
		EntityProperty *properties;
	};
	char *label;			/* label attached to node */
	Vector* outgoing_edges;	/* list of incoming edges (ME)<-(SRC) */
	Vector* incoming_edges;	/* list on outgoing edges (ME)->(DEST) */
} Node;

/* Creates a new node. */
Node* NewNode(long int id, const char *label);

/* Checks if nodes are "equal" */
int Node_Compare(const Node *a, const Node *b);

/* Returns number of edges pointing into node */
int Node_IncomeDegree(const Node *n);

/* Connects source node to destination node by edge */
void Node_ConnectNode(Node* src, Node* dest, struct Edge* e);

/* Adds properties to node
 * prop_count - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void Node_Add_Properties(Node *node, int prop_count, char **keys, SIValue *values);

/* Retrieves node's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue* Node_Get_Property(const Node *node, const char *key);

/* Frees alocated space by given node. */
void FreeNode(Node* node);

#endif
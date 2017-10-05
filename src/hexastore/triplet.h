#ifndef TRIPLET_H
#define TRIPLET_H

#include "../graph/node.h"
#include "../graph/edge.h"
#include "../redismodule.h"
#include "../util/triemap/triemap.h"
#include "../rmutil/sds.h"

#define TRIPLET_ELEMENT_DELIMITER ":"
#define TRIPLET_PREDICATE_DELIMITER "@"

typedef enum {UNKNOW, P, O, OP, S, SP, SO, SOP} TripletKind;
typedef struct {
	Node* subject;
	Edge* predicate;
	Node* object;
	TripletKind kind;
} Triplet;

typedef TrieMapIterator TripletIterator;

/* Creates a new triplet */
Triplet* NewTriplet(Node *s, Edge *p, Node *o);

/* Gets triplet's kind. */
TripletKind TripletGetKind(const Triplet *t);

/* Given an edge (A) -[edge]-> (B), creates a new triplet. */
void TripletFromEdge(Edge *e, Triplet *t);

/* Breaks down triplet into its components */
void TripletComponents(const Triplet *t, char **subject, char **predicate, char **object);

/* Returns a string representation of triplet. */
// char* TripletToString(const Triplet *triplet);
void TripletToString(const Triplet *triplet, sds *str);

/* Frees allocated space by given triplet. */
void FreeTriplet(Triplet *triplet);

// -------------Triplet cursor-------------

// Returns the next triplet from the cursor.
int TripletIterator_Next(TripletIterator* iterator, Triplet** triplet);

void TripletIterator_Free(TripletIterator *cursor);

#endif
#ifndef TRIPLET_H
#define TRIPLET_H

#include "graph/edge.h"
#include "redismodule.h"

typedef enum {UNKNOW, O, P, PO, S, SO, SP, SPO} TripletKind;

typedef struct {
	char* subject;
	char* predicate;
	char* object;
	TripletKind kind;
} Triplet;

typedef struct {
	RedisModuleCtx *ctx;
	RedisModuleKey *key;
	int closed;
} TripletCursor;

// Creates a new triplet
Triplet* NewTriplet(const char* S, const char* P, const char* O);

// Given an edge (A) -[edge]-> (B), creates a new triplet.
Triplet* TripletFromEdge(const Edge* edge);

// Creates a new triplet from string.
Triplet* TripletFromString(const char* input);

// Returns a string representation of triplet.
char* TripletToString(const Triplet* triplet);

// Returns all 6 possible string permutations.
char** GetTripletPermutations(const Triplet* triplet);

// Similar to strcmp,
// Returns <0 if B is greater then A
// Return >0 if A is greater then B
// Return 0 if A equals B
int TripletCompare(const Triplet* A, const Triplet* B);

// Validate checks the triplet for validity and returns false if something is wrong.
int ValidateTriplet(const Triplet* triplet);

// Frees allocated space by given triplet.
void FreeTriplet(Triplet* triplet);

// Triplet cursor

TripletCursor* NewTripletCursor(RedisModuleCtx *ctx, RedisModuleKey* key);

// Returns the next triplet from the cursor.
Triplet* TripletCursorNext(TripletCursor* cursor);

void FreeTripletCursor(TripletCursor* cursor);

#endif
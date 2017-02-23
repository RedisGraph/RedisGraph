#ifndef TRIPLET_H
#define TRIPLET_H

#include "graph/edge.h"
#include "redismodule.h"
#include "util/triemap/triemap.h"

typedef enum {UNKNOW, O, P, PO, S, SO, SP, SPO} TripletKind;

typedef struct {
	char* subject;
	char* predicate;
	char* object;
	TripletKind kind;
} Triplet;

typedef struct {
	TrieMapIterator *iter;
} TripletIterator;

// Creates a new triplet
Triplet* NewTriplet(const char* S, const char* P, const char* O);

// Given an edge (A) -[edge]-> (B), creates a new triplet.
Triplet* TripletFromEdge(const Edge* edge);

// Creates a new triplet from string.
Triplet* TripletFromString(const char* input);

// Returns a string representation of triplet.
char* TripletToString(const Triplet* triplet);

// Checks if given triplets are the same.
int TripletCompare(const Triplet* A, const Triplet* B);

// Validate checks the triplet for validity and returns false if something is wrong.
int ValidateTriplet(const Triplet* triplet);

// Frees allocated space by given triplet.
void FreeTriplet(Triplet* triplet);

// -------------Triplet cursor-------------

TripletIterator* NewTripletIterator(TrieMapIterator *iterator);

// Returns the next triplet from the cursor.
Triplet* TripletIterator_Next(TripletIterator* cursor);

void TripletIterator_Free(TripletIterator* cursor);

#endif
#ifndef TRIPLET_H
#define TRIPLET_H

#include "edge.h"

typedef enum {UNKNOW, O, P, PO, S, SO, SP, SPO} TripletKind;

typedef struct {
	char* subject;
	char* predicate;
	char* object;
	TripletKind kind;
} Triplet;

// Creates a new triplet
Triplet* NewTriplet(const char* S, const char* P, const char* O);

// Given an edge (A) -[edge]-> (B), creates a new triplet.
Triplet* BuildTriplet(const Edge* edge);

// Returns a string representation of triplet.
char* TripletToString(const Triplet* triplet);

// Returns all 6 possible string permutations.
char** GetTripletPermutations(const Triplet* triplet);

// Validate checks the triplet for validity and returns false if something is wrong.
int ValidateTriplet(const Triplet* triplet);

// Frees allocated space by given triplet.
void FreeTriplet(Triplet* triplet);

#endif
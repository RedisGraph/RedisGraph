#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "edge.h"
#include "triplet.h"

int main(int argc, char **argv) {

	Node* src = NewNode("me");
	Node* dest = NewNode("beer");
	Edge* edge = NewEdge(src, dest, "love");

	Triplet* triplet = BuildTriplet(edge);

	FreeEdge(edge);
	FreeNode(src);
	FreeNode(dest);

	assert(strcmp(triplet->subject, "me") == 0);
	assert(strcmp(triplet->predicate, "love") == 0);
	assert(strcmp(triplet->object, "beer") == 0);
	assert(triplet->kind == SPO);

	printf("triplet->kind: %d\n", triplet->kind);

	char* str = TripletToString(triplet);
	printf("str: %s\n", str);
	assert( strcmp(str, "SPO:me:love:beer") == 0);
	
	free(str);
	FreeTriplet(triplet);

	printf("PASS!");
    return 0;
}
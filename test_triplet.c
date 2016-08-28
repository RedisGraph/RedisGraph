#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "edge.h"
#include "triplet.h"


void testTriplet(const char* src, const char* relationship, const char* dest, TripletKind expectedKind, const char* expectedStrRepresentation) {
	Node* srcNode = NewNode(src);
	Node* destNode = NewNode(dest);
	Edge* edge = NewEdge(srcNode, destNode, relationship);

	Triplet* triplet = BuildTriplet(edge);

	FreeEdge(edge);
	FreeNode(srcNode);
	FreeNode(destNode);

	assert(strcmp(triplet->subject, src) == 0);
	assert(strcmp(triplet->predicate, relationship) == 0);
	assert(strcmp(triplet->object, dest) == 0);
	assert(triplet->kind == expectedKind);
	assert(ValidateTriplet(triplet) == 1);
	
	char* str = TripletToString(triplet);
	if(str == 0) {
		assert(str == expectedStrRepresentation);
	} else {
		assert(strcmp(str, expectedStrRepresentation) == 0);
	}
	
	FreeTriplet(triplet);
}

int main(int argc, char **argv) {
	testTriplet("", "", "", UNKNOW, 0);
	testTriplet("me", "", "", S, "SPO:me:");
	testTriplet("", "", "Tokyo", O, "OPS:Tokyo:");
	testTriplet("", "friend with", "", P, "POS:friend with:");
	testTriplet("me", "friend with", "", SP, "SPO:me:friend with:");	
	testTriplet("", "visit", "Tokyo", PO, "POS:visit:Tokyo:");
	testTriplet("me", "", "beer", SO, "SOP:me:beer:");
	testTriplet("me", "love", "beer", SPO, "SPO:me:love:beer");	

	Triplet* triplet = NewTriplet("me", "love", "beer");
	char** permutations = GetTripletPermutations(triplet);

	assert(strcmp(permutations[0], "SPO:me:love:beer") == 0);
	assert(strcmp(permutations[1], "SOP:me:beer:love") == 0);
	assert(strcmp(permutations[2], "PSO:love:me:beer") == 0);
	assert(strcmp(permutations[3], "POS:love:beer:me") == 0);
	assert(strcmp(permutations[4], "OSP:beer:me:love") == 0);
	assert(strcmp(permutations[5], "OPS:beer:love:me") == 0);
	
	// Clean up
	FreeTriplet(triplet);

	for(int i = 0; i < 6; i++) {
		free(permutations[i]);
	}

	free(permutations);

	printf("PASS!");
    return 0;
}
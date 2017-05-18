#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/graph/edge.h"
#include "../src/hexastore/triplet.h"

void testTriplet(const char* src, const char* relationship, const char* dest, TripletKind expectedKind, const char* expectedStrRepresentation) {
	Node* srcNode = NewNode("", src, NULL);
	Node* destNode = NewNode("", dest, NULL);
	Edge* edge = NewEdge("1", NULL, srcNode, destNode, relationship);

	Triplet* triplet = TripletFromEdge(edge);

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
    // testTriplet("", "", "", UNKNOW, 0);
	// testTriplet("me", "", "", S, "SPO:me:");
	// testTriplet("", "", "Tokyo", O, "OPS:Tokyo:");
	// testTriplet("", "friend with", "", P, "POS:friend with:");
	// testTriplet("me", "friend with", "", SP, "SPO:me:friend with:");	
	// testTriplet("", "visit", "Tokyo", PO, "POS:visit:Tokyo:");
	// testTriplet("me", "", "beer", SO, "SOP:me:beer:");
	// testTriplet("me", "love", "beer", SPO, "SPO:me:love:beer"); // Note does not ends with ':'

	// test permutations
	Triplet* triplet = NewTriplet("me", "love", "beer");
	
	// test triplet from string
	Triplet* tripletFromString = TripletFromString("SPO:i:visit:Tokyo");
	assert(tripletFromString != 0);
	assert(tripletFromString->kind == SPO);
	assert(strcmp(tripletFromString->subject, "i") == 0);
	assert(strcmp(tripletFromString->predicate, "visit") == 0);
	assert(strcmp(tripletFromString->object, "Tokyo") == 0);

	FreeTriplet(tripletFromString);

	tripletFromString = TripletFromString("SP:i:visit");
	assert(tripletFromString != 0);
	assert(tripletFromString->kind == SP);
	assert(strcmp(tripletFromString->subject, "i") == 0);
	assert(strcmp(tripletFromString->predicate, "visit") == 0);
	assert(strcmp(tripletFromString->object, "") == 0);

	// test triplet compare
	Triplet* A = NewTriplet("i", "love", "beer");
	Triplet* B = NewTriplet("i", "love", "beer!");
	
	assert(TripletCompare(A, A) == 1);
	assert(TripletCompare(A, B) == 0);
	assert(TripletCompare(B, A) == 0);

	// Clean up
	FreeTriplet(A);
	FreeTriplet(B);
	FreeTriplet(triplet);
	FreeTriplet(tripletFromString);

	printf("PASS!");
    return 0;
}
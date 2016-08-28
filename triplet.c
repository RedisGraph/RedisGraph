#include <stdio.h>
#include "triplet.h"

Triplet* NewTriplet(const char* S, const char* P, const char* O) {
	Triplet* triplet = (Triplet*)malloc(sizeof(Triplet));

	triplet->subject = (char*)malloc(strlen(S) + 1);
	strcpy(triplet->subject, S);

	triplet->predicate = (char*)malloc(strlen(P) + 1);
	strcpy(triplet->predicate, P);

	triplet->object = (char*)malloc(strlen(O) + 1);
	strcpy(triplet->object, O);

	triplet->kind = (strlen(S) > 0) << 2 | (strlen(P) > 0) << 1 | (strlen(O) > 0);

	return triplet;
}

Triplet* BuildTriplet(const Edge* edge) {
	return NewTriplet(edge->src->name, edge->relationship, edge->dest->name);
}

char* TripletToString(const Triplet* triplet) {
	char* str = 0;

	switch(triplet->kind) {
		case O:
			str = (char*)malloc(4 + strlen(triplet->object) + 2);
			sprintf(str, "OPS:%s:", triplet->object);
			break;
		case P:			
			str = (char*)malloc(4 + strlen(triplet->predicate) + 2);
			sprintf(str, "POS:%s:", triplet->predicate);
			break;
		case PO:
			str = (char*)malloc(4 + strlen(triplet->predicate) + 1 + strlen(triplet->object) + 2);
			sprintf(str, "POS:%s:%s:", triplet->predicate, triplet->object);
			break;
		case S:
			str = (char*)malloc(4 + strlen(triplet->subject) + 2);
			sprintf(str, "SPO:%s:", triplet->subject);
			break;
		case SO:
			str = (char*)malloc(4 + strlen(triplet->subject) + 1 + strlen(triplet->object) + 2);
			sprintf(str, "SOP:%s:%s:", triplet->subject, triplet->object);
			break;
		case SP:
			str = (char*)malloc(4 + strlen(triplet->subject) + 1 + strlen(triplet->predicate) + 2);
			sprintf(str, "SPO:%s:%s:", triplet->subject, triplet->predicate);
			break;
		case SPO:
			str = (char*)malloc(4 + strlen(triplet->subject) + 1 + strlen(triplet->predicate) + 1 + strlen(triplet->object) + 2);
			sprintf(str, "SPO:%s:%s:%s", triplet->subject, triplet->predicate, triplet->object);
			break;
		case UNKNOW:
			break;
		
	}

	return str;
}

char** GetTripletPermutations(const Triplet* triplet) {
	char** permutations = (char**)malloc(sizeof(char*) * 6);
	
	int subjectLen = strlen(triplet->subject);
	int predicateLen = strlen(triplet->predicate);
	int objectLen = strlen(triplet->object);
	int bufferSize = 4 + subjectLen + 1 + predicateLen + 1 + objectLen + 1;

	for(int i = 0; i < 6; i++) {
		char* permutation = (char*)malloc(sizeof(char) * bufferSize);
		permutations[i] = permutation;
	}
	
	sprintf(permutations[0], "SPO:%s:%s:%s", triplet->subject, triplet->predicate, triplet->object);
	sprintf(permutations[1], "SOP:%s:%s:%s", triplet->subject, triplet->object, triplet->predicate);
	sprintf(permutations[2], "PSO:%s:%s:%s", triplet->predicate, triplet->subject, triplet->object);
	sprintf(permutations[3], "POS:%s:%s:%s", triplet->predicate, triplet->object, triplet->subject);
	sprintf(permutations[4], "OSP:%s:%s:%s", triplet->object, triplet->subject, triplet->predicate);
	sprintf(permutations[5], "OPS:%s:%s:%s", triplet->object, triplet->predicate, triplet->subject);

	return permutations;
}

int ValidateTriplet(const Triplet* triplet) {
	int valid = 1;	
	switch(triplet->kind) {
		case S:
			if(triplet->subject == 0 || strlen(triplet->subject) == 0) {
				valid = 0;
			}
			if(strlen(triplet->predicate) + strlen(triplet->object) > 0) {
				valid = 0;
			}
			break;
		case O:
			if(triplet->object == 0 || strlen(triplet->object) == 0) {
				valid = 0;
			}
			if(strlen(triplet->predicate) + strlen(triplet->subject) > 0) {
				valid = 0;
			}
			break;
		case P:
			if(triplet->predicate == 0 || strlen(triplet->predicate) == 0) {
				valid = 0;
			}
			if(strlen(triplet->object) + strlen(triplet->subject) > 0) {
				valid = 0;
			}
			break;
		case PO:
			if(triplet->predicate == 0 || strlen(triplet->predicate) == 0) {
				valid = 0;
			}
			if(triplet->object == 0 || strlen(triplet->object) == 0) {
				valid = 0;
			}			
			if(strlen(triplet->subject)) {
				valid = 0;
			}
			break;		
		case SO:
			if(triplet->subject == 0 || strlen(triplet->subject) == 0) {
				valid = 0;
			}
			if(triplet->object == 0 || strlen(triplet->object) == 0) {
				valid = 0;
			}			
			if(strlen(triplet->predicate)) {
				valid = 0;
			}
			break;
		case SP:
			if(triplet->subject == 0 || strlen(triplet->subject) == 0) {
				valid = 0;
			}
			if(triplet->predicate == 0 || strlen(triplet->predicate) == 0) {
				valid = 0;
			}		
			if(strlen(triplet->object)) {
				valid = 0;
			}
			break;
		case SPO:
			if(triplet->subject == 0 || strlen(triplet->subject) == 0) {
				valid = 0;
			}
			if(triplet->predicate == 0 || strlen(triplet->predicate) == 0) {
				valid = 0;
			}
			if(triplet->object == 0 || strlen(triplet->object) == 0) {
				valid = 0;
			}
			break;
		case UNKNOW:
			valid = 1;
			break;
		default:
			valid = 0;
			break;	
	}

	return valid;
}

void FreeTriplet(Triplet* triplet) {
	free(triplet->subject);
	free(triplet->predicate);
	free(triplet->object);
	free(triplet);
}
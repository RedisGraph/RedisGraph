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

void FreeTriplet(Triplet* triplet) {
	free(triplet->subject);
	free(triplet->predicate);
	free(triplet->object);
	free(triplet);
}
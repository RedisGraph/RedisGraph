#include <stdio.h>
#include <string.h>
#include "triplet.h"

Triplet* NewTriplet(const char* S, const char* P, const char* O) {
	Triplet* triplet = (Triplet*)malloc(sizeof(Triplet));

	const char* s = S;
	const char* p = P;
	const char* o = O;

	if(s == 0) {
		s = "";
	}

	if(p == 0) {
		p = "";
	}

	if(o == 0) {
		o = "";
	}

	triplet->subject = (char*)malloc(strlen(s) + 1);
	strcpy(triplet->subject, s);

	triplet->predicate = (char*)malloc(strlen(p) + 1);
	strcpy(triplet->predicate, p);

	triplet->object = (char*)malloc(strlen(o) + 1);
	strcpy(triplet->object, o);

	triplet->kind = (strlen(s) > 0) << 2 | (strlen(p) > 0) << 1 | (strlen(o) > 0);

	return triplet;
}

Triplet* TripletFromEdge(const Edge* edge) {
	return NewTriplet(edge->src->id, edge->relationship, edge->dest->id);
}

Triplet* TripletFromNode(const Node *node) {
	return NewTriplet(node->id, NULL, NULL);
}

// Assuming string format KIND:A:B:C
Triplet* TripletFromString(const char* input) {
	char* kind;
	char* subject = 0;
	char* predicate = 0;
	char* object = 0;

	char* str = (char*)malloc(sizeof(char) * strlen(input) + 1);
	strcpy(str, input);

	char *token = strtok(str, ":");
	kind = token;

	for(int i = 0; i < strlen(kind); i++) {
		switch(kind[i]) {
			case 'S':
				token = strtok(NULL, ":");
				subject = (char*)malloc(sizeof(char) * strlen(token) + 1);
				strcpy(subject, token);
				break;

			case 'P':
				token = strtok(NULL, ":");
				predicate = (char*)malloc(sizeof(char) * strlen(token) + 1);
				strcpy(predicate, token);
				break;

			case 'O':
				token = strtok(NULL, ":");
				object = (char*)malloc(sizeof(char) * strlen(token) + 1);
				strcpy(object, token);
				break;

			default:
				break;
		}
	}

	Triplet* t = NewTriplet(subject, predicate, object);

	free(str);
	if(subject) {
		free(subject);
	}
	if(predicate) {
		free(predicate);
	}
	if(object) {
		free(object);
	}

	return t;
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

// Assuming Triplets are of the same kind.
int TripletCompare(const Triplet* A, const Triplet* B) {
	return ((strcmp(A->subject, B->subject) == 0) &&
		(strcmp(A->predicate, B->predicate) == 0) &&
		(strcmp(A->object, B->object) == 0));
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

TripletIterator* NewTripletIterator(TrieMapIterator *iter) {
	TripletIterator* iterator = (TripletIterator*)malloc(sizeof(TripletIterator));
	iterator->iter = iter;
	return iterator;
}

// Returns the next triplet from the cursor
// or NULL when cursor is depleted
Triplet* TripletIterator_Next(TripletIterator* iterator) {
	char *key = NULL;
	tm_len_t len = 0;
	void *ptr = NULL;
	int res = TrieMapIterator_Next(iterator->iter, &key, &len, &ptr);
	if(res == 0) {
		return NULL;
	}

	Triplet* t = TripletFromString(key);
	return t;
}

void TripletIterator_Free(TripletIterator* iterator) {
	TrieMapIterator_Free(iterator->iter);
	free(iterator);
}
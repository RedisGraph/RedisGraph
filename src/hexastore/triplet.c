#include <stdio.h>
#include <string.h>
#include "triplet.h"

Triplet* NewTriplet(const char *S, const char *P, const char *O) {
	Triplet* triplet = (Triplet*)malloc(sizeof(Triplet));

	const char *s = (S==NULL) ? "" : S;
	const char *p = (P==NULL) ? "" : P;
	const char *o = (O==NULL) ? "" : O;
	
	triplet->subject = strdup(s);
	triplet->predicate = strdup(p);
	triplet->object = strdup(o);
	triplet->kind = (strlen(s) > 0) << 2 | (strlen(o) > 0) << 1 | (strlen(p) > 0);

	return triplet;
}

Triplet* TripletFromEdge(const Edge *edge) {
	const char *S = edge->src->id;
	const char *O = edge->dest->id;
	char *P = NULL;

	// Predicate is composed of edge label and edge id.
	if(edge->relationship != NULL) {
		if(edge->id != NULL) {
			asprintf(&P, "%s%s%s", edge->relationship, TRIPLET_PREDICATE_DELIMITER, edge->id);
		} else {
			asprintf(&P, "%s%s", edge->relationship, TRIPLET_PREDICATE_DELIMITER);
		}
	}

	Triplet *t = NewTriplet(S, P, O);
	free(P);

	return t;
}

Triplet* TripletFromNode(const Node *node) {
	return NewTriplet(node->id, NULL, NULL);
}

// Assuming string format KIND:A:B:C
Triplet* TripletFromString(const char *input) {
	char *kind;
	char *subject = NULL;
	char *predicate = NULL;
	char *object = NULL;

	char *str = strdup(input);
	char *token = strtok(str, TRIPLET_ELEMENT_DELIMITER);
	kind = token;

	for(int i = 0; i < strlen(kind); i++) {
		token = strtok(NULL, TRIPLET_ELEMENT_DELIMITER);
		switch(kind[i]) {
			case 'S':
				subject = strdup(token);
				break;

			case 'P':
				predicate = strdup(token);
				break;

			case 'O':
				object = strdup(token);
				break;

			default:
				break;
		}
	}

	Triplet *t = NewTriplet(subject, predicate, object);

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

char* TripletToString(const Triplet *triplet) {
	char *str = NULL;

	switch(triplet->kind) {
		case S:
			asprintf(&str, "SPO:%s", triplet->subject);
			break;
		case P:			
			asprintf(&str, "POS:%s", triplet->predicate);
			break;
		case O:
			asprintf(&str, "OPS:%s", triplet->object);
			break;
		case OP:
			asprintf(&str, "OPS:%s:%s", triplet->object, triplet->predicate);
			break;
		case SO:
			asprintf(&str, "SOP:%s:%s", triplet->subject, triplet->object);
			break;
		case SP:
			asprintf(&str, "SPO:%s:%s", triplet->subject, triplet->predicate);
			break;
		case SOP:
			asprintf(&str, "SOP:%s:%s:%s", triplet->subject, triplet->object, triplet->predicate);
			break;
		case UNKNOW:
			break;
		
	}

	return str;
}

void TripletToGraph(const Triplet *triplet, Node *src, Edge *edge, Node *dest) {
	// Copy triplet subject to src id
	if(strcmp(triplet->subject, "") == 0) {
		src->id = NULL;
	} else {
		src->id = strdup(triplet->subject);
	}
	
	// Copy triplet predicate to edge label and id
	if(strcmp(triplet->predicate, "") == 0) {
		edge->id = NULL;
		edge->relationship = NULL;
	} else {
		char *p = strdup(triplet->predicate);
		edge->id = strdup(strtok(p, TRIPLET_PREDICATE_DELIMITER));
		edge->relationship = strdup(strtok(NULL, TRIPLET_PREDICATE_DELIMITER));
		free(p);
	}
	
	// Copy triplet object to dest id
	if(strcmp(triplet->object, "") == 0) {
		dest->id = NULL;
	} else {
		dest->id = strdup(triplet->object);
	}
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
		case OP:
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
		case SOP:
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

// Returns the next triplet from the cursor
// or NULL when cursor is depleted
int TripletIterator_Next(TripletIterator* iterator, Triplet** triplet) {
	char *key = NULL;
	tm_len_t len = 0;
	return TrieMapIterator_Next(iterator, &key, &len, triplet);
}

void TripletIterator_Free(TripletIterator* iterator) {
	TrieMapIterator_Free(iterator);
}
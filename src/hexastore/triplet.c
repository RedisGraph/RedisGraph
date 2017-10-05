#include <stdio.h>
#include <string.h>
#include "triplet.h"
#include "assert.h"

TripletKind TripletGetKind(const Triplet *t) {
	int s = (t->subject != NULL && t->subject->id != INVALID_ENTITY_ID);
	int o = (t->object != NULL && t->object->id != INVALID_ENTITY_ID);
	int p = (t->predicate != NULL && t->predicate->relationship != NULL);
	
	TripletKind kind = (s > 0) << 2 | (o > 0) << 1 | (p > 0);
	return kind;
}

Triplet* NewTriplet(Node *s, Edge *p, Node *o) {
	Triplet* triplet = (Triplet*)malloc(sizeof(Triplet));
	triplet->subject = s;
	triplet->predicate = p;
	triplet->object = o;

	char *subject, *predicate, *object;
	triplet->kind = TripletGetKind(triplet);

	return triplet;
}

void TripletFromEdge(Edge *e, Triplet *t) {
	t->subject = e->src;
	t->predicate = e;
	t->object = e->dest;
	t->kind = TripletGetKind(t);
}

void TripletComponents(const Triplet *t, char **subject, char **predicate, char **object) {
	if(t->subject == NULL || t->subject->id == INVALID_ENTITY_ID) {
		*subject = "";
	} else {
		asprintf(subject, "%ld", t->subject->id);
	}
	
	if(t->object == NULL || t->object->id == INVALID_ENTITY_ID) {
		*object = "";
	} else {
		asprintf(object, "%ld", t->object->id);
	}
	
	if(t->predicate == NULL) {
		*predicate = "";
	} else if(t->predicate->id == INVALID_ENTITY_ID) {
		asprintf(predicate, "%s%s", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER);
	} else {
		asprintf(predicate, "%s%s%ld", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, t->predicate->id);
	}
}

void TripletToString(const Triplet *triplet, sds *str) {
	/* Sanity, atleast one component should not be NULL. */
	assert(triplet->subject != NULL || triplet->predicate != NULL || triplet->object != NULL);

	/* Clear sds, maintains sds allocation size. */
	(*str)[0] = '\0';
	sdsupdatelen(*str);

	switch(triplet->kind) {
		case S:
			*str = sdscatprintf(*str, "SPO:%ld", triplet->subject->id);
			break;
		case P:			
			if(triplet->predicate->id == INVALID_ENTITY_ID) {
				*str = sdscatprintf(*str, "POS:%s%s", triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER);
			} else {
				*str = sdscatprintf(*str, "POS:%s%s%ld", triplet->predicate->relationship,
								   TRIPLET_PREDICATE_DELIMITER, triplet->predicate->id);
			}
			break;
		case O:
			*str = sdscatprintf(*str, "OPS:%ld", triplet->object->id);
			break;
		case OP:
			if(triplet->predicate->id == INVALID_ENTITY_ID) {
				*str = sdscatprintf(*str, "OPS:%ld:%s%s", triplet->object->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER);
			} else {
				*str = sdscatprintf(*str, "OPS:%ld:%s%s%ld", triplet->object->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, triplet->predicate->id);
			}
			break;
		case SO:
			*str = sdscatprintf(*str, "SOP:%ld:%ld", triplet->subject->id, triplet->object->id);
			break;
		case SP:
			if(triplet->predicate->id == INVALID_ENTITY_ID) {
				*str = sdscatprintf(*str, "SPO:%ld:%s%s", triplet->subject->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER);
			} else {
				*str = sdscatprintf(*str, "SPO:%ld:%s%s%ld", triplet->subject->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, triplet->predicate->id);
			}
			break;
		case SOP:
			if(triplet->predicate->id == INVALID_ENTITY_ID) {
				*str = sdscatprintf(*str, "SOP:%ld:%ld:%s%s", triplet->subject->id,
								   triplet->object->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER);
			} else {
				*str = sdscatprintf(*str, "SOP:%ld:%ld:%s%s%ld", triplet->subject->id,
								   triplet->object->id,
								   triplet->predicate->relationship, TRIPLET_PREDICATE_DELIMITER,
								   triplet->predicate->id);
			}
			break;
		case UNKNOW:
			break;
		
	}
}

void FreeTriplet(Triplet* triplet) {
	if(triplet == NULL) return;
	free(triplet);
	triplet = NULL;
}

/* Returns the next triplet from the cursor
 * or NULL when cursor is depleted. */
int TripletIterator_Next(TripletIterator* iterator, Triplet** triplet) {
	char *key = NULL;
	tm_len_t len = 0;
	return TrieMapIterator_Next(iterator, &key, &len, (void**)triplet);
}

void TripletIterator_Free(TripletIterator* iterator) {
	TrieMapIterator_Free(iterator);
}
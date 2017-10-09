#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/util/prng.h"
#include "../../src/rmutil/sds.h"
#include "../../src/graph/edge.h"
#include "../../src/hexastore/triplet.h"

void test_triplet_creation() {
	Node *subject_node = NewNode(get_new_id(), "actor");
	Node *object_node = NewNode(get_new_id(), "movie");
	Edge *predicate_edge = NewEdge(get_new_id(), subject_node, object_node, "act");
	Triplet *triplet = NewTriplet(subject_node, predicate_edge, object_node);

	assert(triplet);
	assert(triplet->subject == subject_node);
	assert(triplet->object == object_node);
	assert(triplet->predicate == predicate_edge);
	assert(triplet->kind == SOP);
	
	FreeTriplet(triplet);

	Triplet t;
	TripletFromEdge(predicate_edge, &t);

	assert(t.subject == subject_node);
	assert(t.object == object_node);
	assert(t.predicate == predicate_edge);
	assert(t.kind == SOP);
}

void test_triplet_string_rep() {
	char *subject;
	char *predicate;
	char *object;	
	char* subject_node_id_str;
	char* object_node_id_str;
	char* predicate_edge_id_str;
	char *expected_triplet_str;
	Triplet *triplet;

	Node *subject_node = NewNode(get_new_id(), "actor");
	Node *object_node = NewNode(get_new_id(), "movie");
	Edge *predicate_edge = NewEdge(get_new_id(), subject_node, object_node, "act");
	
	/* #########################################################################
	 * Triplet with all three components.
	 * ######################################################################### */
	
	triplet = NewTriplet(subject_node, predicate_edge, object_node);
	
	asprintf(&subject_node_id_str, "%ld", subject_node->id);
	asprintf(&object_node_id_str, "%ld", object_node->id);
	asprintf(&predicate_edge_id_str, "%s%s%ld", predicate_edge->relationship,
			 TRIPLET_PREDICATE_DELIMITER, predicate_edge->id);

	TripletComponents(triplet, &subject, &predicate, &object);
	assert(strcmp(subject, subject_node_id_str) == 0);
	assert(strcmp(object, object_node_id_str) == 0);
	assert(strcmp(predicate, predicate_edge_id_str) == 0);

	asprintf(&expected_triplet_str, "SOP:%ld:%ld:%s%s%ld", subject_node->id, object_node->id,
			 predicate_edge->relationship, TRIPLET_PREDICATE_DELIMITER, predicate_edge->id);
	
	sds sds_triplet = sdsempty();
	TripletToString(triplet, &sds_triplet);
	assert(strcmp(sds_triplet, expected_triplet_str) == 0);

	FreeTriplet(triplet);

	/* #########################################################################
	 * Triplet without an object.
	 * ######################################################################### */
	
	triplet = NewTriplet(subject_node, predicate_edge, NULL);
	assert(triplet->kind == SP);
		
	asprintf(&subject_node_id_str, "%ld", subject_node->id);
	asprintf(&predicate_edge_id_str, "%s%s%ld", predicate_edge->relationship,
			 TRIPLET_PREDICATE_DELIMITER, predicate_edge->id);
	
	TripletComponents(triplet, &subject, &predicate, &object);
	assert(strcmp(subject, subject_node_id_str) == 0);
	assert(strcmp(object, "") == 0);
	assert(strcmp(predicate, predicate_edge_id_str) == 0);
	
	asprintf(&expected_triplet_str, "SPO:%ld:%s%s%ld", subject_node->id, predicate_edge->relationship,
			 TRIPLET_PREDICATE_DELIMITER, predicate_edge->id);
	
	TripletToString(triplet, &sds_triplet);
	assert(strcmp(sds_triplet, expected_triplet_str) == 0);

	FreeTriplet(triplet);

	/* #########################################################################
	 * Triplet without a predicate.
	 * ######################################################################### */
	
	triplet = NewTriplet(subject_node, NULL, object_node);
	assert(triplet->kind == SO);
		
	asprintf(&subject_node_id_str, "%ld", subject_node->id);
	asprintf(&object_node_id_str, "%ld", object_node->id);
	
	TripletComponents(triplet, &subject, &predicate, &object);
	assert(strcmp(subject, subject_node_id_str) == 0);
	assert(strcmp(object, object_node_id_str) == 0);
	assert(strcmp(predicate, "") == 0);
	
	asprintf(&expected_triplet_str, "SOP:%ld:%ld", subject_node->id, object_node->id);

	TripletToString(triplet, &sds_triplet);
	assert(strcmp(sds_triplet, expected_triplet_str) == 0);

	FreeTriplet(triplet);
	/* #########################################################################
	 * Triplet with an id less predicate.
	 * ######################################################################### */
	
	predicate_edge->id = INVALID_ENTITY_ID;
	triplet = NewTriplet(subject_node, predicate_edge, object_node);
	assert(triplet->kind == SOP);
		
	asprintf(&subject_node_id_str, "%ld", subject_node->id);
	asprintf(&object_node_id_str, "%ld", object_node->id);
	asprintf(&predicate_edge_id_str, "%s%s", predicate_edge->relationship,
			 TRIPLET_PREDICATE_DELIMITER);
	
	TripletComponents(triplet, &subject, &predicate, &object);
	assert(strcmp(subject, subject_node_id_str) == 0);
	assert(strcmp(object, object_node_id_str) == 0);
	assert(strcmp(predicate, predicate_edge_id_str) == 0);
	
	asprintf(&expected_triplet_str, "SOP:%s:%s:%s", subject_node_id_str, object_node_id_str,
			 predicate_edge_id_str);

	TripletToString(triplet, &sds_triplet);
	assert(strcmp(sds_triplet, expected_triplet_str) == 0);

	FreeTriplet(triplet);

	/* #########################################################################
	 * Triplet with an id less subject and predicate.
	 * ######################################################################### */
	
	subject_node->id = INVALID_ENTITY_ID;
	predicate_edge->id = INVALID_ENTITY_ID;

	triplet = NewTriplet(subject_node, predicate_edge, object_node);
	assert(triplet->kind == OP);
		
	subject_node_id_str = "";
	asprintf(&object_node_id_str, "%ld", object_node->id);
	asprintf(&predicate_edge_id_str, "%s%s", predicate_edge->relationship,
			 TRIPLET_PREDICATE_DELIMITER);
	
	TripletComponents(triplet, &subject, &predicate, &object);
	assert(strcmp(subject, subject_node_id_str) == 0);
	assert(strcmp(object, object_node_id_str) == 0);
	assert(strcmp(predicate, predicate_edge_id_str) == 0);
	
	asprintf(&expected_triplet_str, "OPS:%s:%s", object_node_id_str,
			 predicate_edge_id_str);

	TripletToString(triplet, &sds_triplet);
	assert(strcmp(sds_triplet, expected_triplet_str) == 0);

	FreeTriplet(triplet);
}

int main(int argc, char **argv) {
	test_triplet_creation();
	test_triplet_string_rep();
	printf("test_triplet - PASS!\n");
    return 0;
}
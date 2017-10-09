#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/util/prng.h"
#include "../../src/hexastore/hexastore.h"
#include "../../src/hexastore/triplet.h"

void test_hexastore() {
    Triplet *t;
    TripletIterator *it;
    long int id;

    id = get_new_id();
    Node *subject_node = NewNode(id, "actor");
	id = get_new_id();
    Node *object_node = NewNode(id, "movie");
	id = get_new_id();
    Edge *predicate_edge = NewEdge(id, subject_node, object_node, "act");
    Triplet *triplet = NewTriplet(subject_node, predicate_edge, object_node);

	HexaStore *hexastore = _NewHexaStore();
    assert(hexastore);
    
    HexaStore_InsertAllPerm(hexastore,  triplet);
    assert(hexastore->cardinality == 6);

    /* Note re-introducing the same triplet will seg-fault
     * this is because the same triplet is used six times.
     * i.e. freed six times */

    /* Search hexastore.
     * Scan entire hexastore. */
    it = HexaStore_Search(hexastore, "");
    for(int i = 0; i < 6; i++) {        
        assert(TripletIterator_Next(it, &t));
        assert(t == triplet);
    }
    assert(!TripletIterator_Next(it, &t));

    /* Searching all possible permutations. */
    it = HexaStore_Search(hexastore, "SPO");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    it = HexaStore_Search(hexastore, "SOP");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    it = HexaStore_Search(hexastore, "PSO");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    it = HexaStore_Search(hexastore, "POS");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    it = HexaStore_Search(hexastore, "OSP");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    it = HexaStore_Search(hexastore, "OPS");
    assert(TripletIterator_Next(it, &t));
    assert(!TripletIterator_Next(it, &t));

    HexaStore_RemoveAllPerm(hexastore, triplet);
    assert(hexastore->cardinality == 0);

    /* Searching an empty hexastore */
    it = HexaStore_Search(hexastore, "");
    assert(!TripletIterator_Next(it, &t));
}

int main(int argc, char **argv) {
	test_hexastore();
    printf("test_hexastore - PASS!\n");
	return 0;
}
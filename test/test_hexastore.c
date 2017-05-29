#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/hexastore/hexastore.h"
#include "../src/hexastore/triplet.h"


void test_hexastore() {
    HexaStore *hexastore = _NewHexaStore();
    Triplet *triplet = NewTriplet("Michael", "Works", "Dunder Mifflin");

    HexaStore_InsertTriplet(hexastore, triplet);
    TripletIterator *iter = HexaStore_Search(hexastore, "SPO:Michael:Works:");
    
    Triplet *item = NULL;
    
    int res = TripletIterator_Next(iter, &item);
    assert(item != NULL);
    assert(res);

    res = TripletIterator_Next(iter, &item);
    assert(!res);
    
    ////////////////////////////////////////////////////////////////////////////////

    iter = HexaStore_Search(hexastore, "S");
    item = NULL;
    
    // SOP, SPO
    for(int i = 0; i < 2; i++) {
        res = TripletIterator_Next(iter, &item);
        assert(item != NULL);
        assert(res);
    }

    res = TripletIterator_Next(iter, &item);
    assert(!res);

    ////////////////////////////////////////////////////////////////////////////////

    iter = HexaStore_Search(hexastore, "O");
    item = NULL;
    
    // OPS, OSP
    for(int i = 0; i < 2; i++) {
        res = TripletIterator_Next(iter, &item);
        assert(item != NULL);
        assert(res);
    }

    res = TripletIterator_Next(iter, &item);
    assert(!res);
    
    ////////////////////////////////////////////////////////////////////////////////

    iter = HexaStore_Search(hexastore, "P");
    item = NULL;
    
    // PSO, POS
    for(int i = 0; i < 2; i++) {
        res = TripletIterator_Next(iter, &item);
        assert(item != NULL);
        assert(res);
    }

    res = TripletIterator_Next(iter, &item);
    assert(!res);

    ////////////////////////////////////////////////////////////////////////////////

    Triplet *t = NewTriplet("Michael", NULL, NULL);

    iter = HexaStore_QueryTriplet(hexastore, t);
    item = NULL;
    
    res = TripletIterator_Next(iter, &item);
    assert(item != NULL);
    assert(res);

    res = TripletIterator_Next(iter, &item);
    assert(!res);

    ////////////////////////////////////////////////////////////////////////////////
    
	printf("PASS!");
}

int main(int argc, char **argv) {
	test_hexastore();
	return 0;
}
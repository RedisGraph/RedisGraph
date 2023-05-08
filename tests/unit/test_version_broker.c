/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "src/util/rmalloc.h"
#include "src/mvcc/version_broker.h"

void setup();

#define TEST_INIT setup();
#include "acutest.h"

void setup() {
	printf("setup\n");
	// use the malloc family for allocations
	Alloc_Reset();
}

void test_VersionBroker_New(void) {
	VersionBroker vb = VersionBroker_New();
	TEST_ASSERT(vb != NULL);

	// get all active versions
	// expecting a single version, the initial version 0
	int64_t *versions   = NULL;
	int64_t *ref_counts = NULL;
	int64_t *obj_counts = NULL;
	uint n = VersionBroker_Versions(vb, &versions, &ref_counts, &obj_counts);

	TEST_ASSERT(n             == 1);  // single version
	TEST_ASSERT(versions[0]   == 0);  // version 0
	TEST_ASSERT(ref_counts[0] == 1);  // ref count 1
	TEST_ASSERT(obj_counts[0] == 0);  // no objects

	VersionBroker_Print(vb);

	// clean up
	rm_free(versions);
	rm_free(ref_counts);
	rm_free(obj_counts);
	VersionBroker_Free(vb);
}

TEST_LIST = {
	{ "VersionBroker_New", test_VersionBroker_New},
	{ NULL, NULL }
};


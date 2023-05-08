/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdint.h>

typedef struct VersionBroker_opaque *VersionBroker;

// object free callback
typedef void (*free_cb)(void *);

// create a new version broker
VersionBroker VersionBroker_New(void);

// should be called by a READ query
// sets read version within caller's thread local storage
// returns current READ version
int64_t VersionBroker_GetReadVersion
(
	VersionBroker vb  // version broker to get READ version from
);

// counterpart to GetReadVersion
// a READ thread done processing should "return" its read version
void VersionBroker_ReturnReadVersion
(
	VersionBroker vb  // version broker
);

// should be called by a WRITE query
// sets write version within caller's thread local storage
// returns current WRITE version
int64_t VersionBroker_GetWriteVersion
(
	VersionBroker vb  // version broker
);

// counterpart to GetWriteVersion
// a WRITE thread done processing should "return" its write version
void VersionBroker_ReturnWriteVersion
(
	VersionBroker vb  // version broker
);

// registers object under thread's version
// free callback will be called on object once the version is finalized
void VersionBroker_RegisterObj
(
	VersionBroker vb,  // version broker
	void *obj,         // object associated with version
	free_cb cb         // object free function
);

// free version broker
void VersionBroker_Free
(
	VersionBroker vb  // version broker to free
);


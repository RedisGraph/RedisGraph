/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "../util/arr.h"
#include "./version_broker.h"

#include <pthread.h>
#include <stdatomic.h>

pthread_key_t tls_mvcc_version;

// versioned object
// an object which is owned by a version
// once the version is finalized the free callback
// will be called on the object
typedef struct {
	void *obj;   // object associated with version
	free_cb cb;  // object's free callback
} VersionedObj;

// version descriptor
typedef struct {
	int64_t v;              // version
	atomic_int ref_count;   // version reference count
	VersionedObj *objects;  // list of objects associated with version
} Version;

// version broker
// manages all currently active versions
struct VersionBroker_opaque
{
	volatile int64_t latest_version;  // latest active version
	Version *active_versions;         // active versions
	pthread_rwlock_t rwlock;          // lock
};

// locates version 'v' within version-broker active versions
static Version *_GetVersion
(
	const VersionBroker vb,  // version broker
	int64_t v,               // version number to get
	int *idx                 // version index
) {
	ASSERT(vb != NULL);
	ASSERT(v  <= vb->version);

	int i = 0;
	Version *_v = NULL;
	int n = array_len(vb->active_versions);

	// scan through all active versions
	for(; i < n; i++) {
		if((vb->active_versions + i)->v == v) {
			// version located
			_v = vb->active_versions + i;
			break;
		}
	}

	// expecting to find version
	ASSERT(_v != NULL);

	// caller asked for version index
	if(idx != NULL) {
		*idx = i;
	}

	return _v;
}

// add a new version
static Version *_AddVersion
(
	VersionBroker vb,  // version broker
	int64_t v          // new version to add
) {
	ASSERT(vb != NULL);
	ASSERT(_GetVersion(vb, v, NULL) == NULL);

	// create and add a new version
	Version _v = {v, ATOMIC_VAR_INIT(0), array_new(VersionedObj, 1)};

	// acquire WRITE lock
	pthread_rwlock_wrlock(&vb->rwlock);

	array_append(vb->active_versions, _v);

	// release WRITE lock
	pthread_rwlock_unlock(&vb->rwlock);

	// return newly created version
	return vb->active_versions + (array_len(vb->active_versions) -1);
}

// finalize version 'v'
// call free callback for each object associated with version
// removes version 'v' from active versions
static void _FinalizeVersion
(
	VersionBroker vb,  // version broker
	int64_t v          // version to finalize
) {
	// acquire write lock
	pthread_rwlock_wrlock(&vb->rwlock);

	// locate version
	int i = -1;
	Version *_v = _GetVersion(vb, v, &i);
	ASSERT(_v->ref_count == 0);

	//--------------------------------------------------------------------------
	// release version
	//--------------------------------------------------------------------------

	// remove version from active versions
	array_del_fast(vb->active_versions, i);

	// release write lock
	pthread_rwlock_unlock(&vb->rwlock);
	
	//--------------------------------------------------------------------------
	// free each registered version object
	//--------------------------------------------------------------------------

	int n = array_len(_v->objects);
	for(int i = 0; i < n; i++) {
		VersionedObj *vo = _v->objects + i;
		vo->cb(vo->obj);
	}
	array_free(_v->objects);
}

// should be called only once!
void VersionBroker_Init(void) {
	// initialize thread-local mvcc version storage
	int res = pthread_key_create(&tls_mvcc_version, NULL);
	ASSERT(res == 0);
}

// create a new version broker
VersionBroker VersionBroker_New(void) {
	VersionBroker vb = rm_malloc(sizeof(struct VersionBroker_opaque));
	
	vb->latest_version = 0;  // starting with version 0
	vb->active_versions = array_new(Version, 1);

	pthread_rwlock_init(&vb->rwlock, NULL);

	// create first version
	_AddVersion(vb, vb->latest_version);

	return vb;
}

// should be called by a READ query
// sets read version within caller's thread local storage
// returns current READ version
int64_t VersionBroker_GetReadVersion
(
	VersionBroker vb  // version broker to get READ version from
) {
	ASSERT(vb != NULL);

	// acquire READ lock
	pthread_rwlock_rdlock(&vb->rwlock);

	// current active version
	int64_t v = vb->latest_version;

	// locate version and increase ref count
	Version *_v = _GetVersion(vb, v, NULL);
	ASSERT(_v != NULL);

	atomic_fetch_add(&_v->ref_count, 1);

	// release READ lock
	pthread_rwlock_unlock(&vb->rwlock);

	// set thread local storage
	pthread_setspecific(tls_mvcc_version, (void*)v);

	return v;
}

// counterpart to GetReadVersion
// a READ thread done processing should "return" its read version
void VersionBroker_ReturnReadVersion
(
	VersionBroker vb  // version broker
) {
	ASSERT(vb != NULL);
	
	// get thread mvcc version
	int64_t v = (int64_t)pthread_getspecific(tls_mvcc_version);

	// acquire READ lock
	pthread_rwlock_rdlock(&vb->rwlock);

	// decrease version reference count
	Version *_v = _GetVersion(vb, v, NULL);
	int ref_count = atomic_fetch_sub(&_v->ref_count, 1);
	ASSERT(ref_count >= 0);

	// release READ lock
	pthread_rwlock_unlock(&vb->rwlock);

	// finalize version if reference count reached 0
	if(ref_count == 0) {
		_FinalizeVersion(vb, v);
	}
}

// should be called by a WRITE query
// sets write version within caller's thread local storage
// returns current WRITE version
int64_t VersionBroker_GetWriteVersion
(
	VersionBroker vb  // version broker
) {
	ASSERT(vb != NULL);

	// write version must be greater then latest version
	int64_t v = vb->latest_version + 1;

	// create new version
	Version *_v = _AddVersion(vb, v);

	// increase version's ref count
	atomic_fetch_add(&_v->ref_count, 1);

	// set thread local storage
	pthread_setspecific(tls_mvcc_version, (void*)v);

	return v;
}

// counterpart to GetWriteVersion
// a WRITE thread done processing should "return" its write version
void VersionBroker_ReturnWriteVersion
(
	VersionBroker vb  // version broker
) {
	ASSERT(vb != NULL);
	
	// get thread mvcc version
	int64_t v = (int64_t)pthread_getspecific(tls_mvcc_version);
	ASSERT(v == vb->latest_version + 1);

	// update latest version
	vb->latest_version = v;

	// acquire READ lock
	pthread_rwlock_rdlock(&vb->rwlock);

	// decrease prev version reference count
	Version *_prev_v = _GetVersion(vb, v - 1, NULL);
	int prev_ref_count = atomic_fetch_sub(&_prev_v->ref_count, 1);
	ASSERT(ref_count >= 0);

	// release READ lock
	pthread_rwlock_unlock(&vb->rwlock);

	// finalize version if reference count reached 0
	if(prev_ref_count == 0) {
		_FinalizeVersion(vb, v - 1);
	}
}

// registers object under thread's version
// free callback will be called on object once the version is finalized
void VersionBroker_RegisterObj
(
	VersionBroker vb,  // version broker
	void *obj,         // object associated with version
	free_cb cb         // object free function
) {
	ASSERT(vb  != NULL);
	ASSERT(cb  != NULL);
	ASSERT(obj != NULL);

	// get thread mvcc version
	int64_t v = (int64_t)pthread_getspecific(tls_mvcc_version);

	// acquire READ lock
	pthread_rwlock_rdlock(&vb->rwlock);

	// get version descriptor
	Version *_v = _GetVersion(vb, v, NULL);

	// release READ lock
	pthread_rwlock_unlock(&vb->rwlock);

	// register object with version
	VersionedObj vo = {obj, cb};
	array_append(_v->objects, vo);
}

// free version broker
void VersionBroker_Free
(
	VersionBroker vb  // version broker to free
) {
	ASSERT(vb != NULL);

	array_free(vb->active_versions);

	pthread_rwlock_destroy(&vb->rwlock);

	rm_free(vb);
}


/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "sipath.h"
#include "../util/rmalloc.h"
#include "../util/arr.h"
#include "array.h"

SIValue SIPath_New(Path p) {
	SIValue path;
	path.ptrval = rm_malloc(sizeof(Path));
	Path *pathPtr = (Path *)path.ptrval;
	*pathPtr = Path_Clone(p);
	path.type = T_PATH;
	path.allocation = M_SELF;
	return path;
}

SIValue SIPath_Clone(SIValue p) {
	Path *pathPtr = (Path *)p.ptrval;
	return SIPath_New(*pathPtr);
}

SIValue SIPath_Relationships(SIValue p) {
	Path path = *(Path *) p.ptrval;
	uint edgeCount = Path_EdgeCount(path);
	SIValue array = SIArray_New(edgeCount);
	for(uint i = 0; i < edgeCount; i++) {
		SIArray_Append(&array, SI_Edge(&path.edges[i]));
	}
	return array;
}

SIValue SIPath_GetRelationship(SIValue p, size_t i) {
	assert(i < SIPath_Length(p) && i > 0);
	Path path = *(Path *) p.ptrval;
	return SI_Edge(&path.edges[i]);
}

SIValue SIPath_Nodes(SIValue p) {
	Path path = *(Path *) p.ptrval;
	uint nodeCount = Path_NodeCount(path);
	SIValue array = SIArray_New(nodeCount);
	for(uint i = 0; i < nodeCount; i++) {
		SIArray_Append(&array, SI_Node(&path.nodes[i]));
	}
	return array;
}

SIValue SIPath_GetNode(SIValue p, size_t i) {
	assert(i < SIPath_Length(p) + 1 && i > 0);
	Path path = *(Path *) p.ptrval;
	return SI_Node(&path.nodes[i]);
}

size_t SIPath_Length(SIValue p) {
	Path path = *(Path *) p.ptrval;
	return Path_Len(path);
}

size_t SIPath_Size(SIValue p) {
	Path path = *(Path *) p.ptrval;
	return Path_Size(path);
}

size_t SIPath_NodeCount(SIValue p) {
	Path path = *(Path *) p.ptrval;
	return Path_NodeCount(path);
}

size_t SIPath_EdgeCount(SIValue p) {
	Path path = *(Path *) p.ptrval;
	return Path_EdgeCount(path);
}

XXH64_hash_t SIPath_HashCode(SIValue p) {
	SIType t = SI_TYPE(p);
	XXH64_hash_t hashCode = XXH64(&t, sizeof(t), 0);
	size_t nodeCount = SIPath_NodeCount(p);
	for(size_t i = 0; i < nodeCount - 1 ; i++) {
		SIValue node = SIPath_GetNode(p, i);
		hashCode = 31 * hashCode + SIValue_HashCode(node);
		SIValue edge = SIPath_GetRelationship(p, i);
		hashCode = 31 * hashCode + SIValue_HashCode(edge);
	}
	if(nodeCount > 0) {
		SIValue node = SIPath_GetNode(p, nodeCount - 1);
		hashCode = 31 * hashCode + SIValue_HashCode(node);
	}
	return hashCode;
}

void SIPath_ToString(SIValue p, char **buf, size_t *bufferLen, size_t *bytesWritten) {

	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// open path with "["
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "[");

	size_t nodeCount = SIPath_NodeCount(p);
	for(size_t i = 0; i < nodeCount - 1; i ++) {
		// write the next value
		SIValue node = SIPath_GetNode(p, i);
		SIValue_ToString(node, buf, bufferLen, bytesWritten);
		* bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
		SIValue edge = SIPath_GetRelationship(p, i);
		SIValue_ToString(edge, buf, bufferLen, bytesWritten);
		* bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}
	if(nodeCount > 0) {
		SIValue node = SIPath_GetNode(p, nodeCount - 1);
		SIValue_ToString(node, buf, bufferLen, bytesWritten);
	}

	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// close array with "]"
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "]");
}

void SIPath_Free(SIValue p) {
	if(p.allocation == M_SELF) {
		Path path = *(Path *) p.ptrval;
		Path_Free(path);
		rm_free(p.ptrval);
	}
}

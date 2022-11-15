/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <string.h>
#include <ctype.h>
#include "RG.h"
#include "rmalloc.h"

void str_tolower(const char *str, char *lower, size_t *lower_len) {
	size_t str_len = strlen(str);
	//Avoid overflow
	ASSERT(*lower_len >= str_len);

	//Update lower len
	*lower_len = str_len;

	size_t i = 0;
	for(; i < str_len; i++) lower[i] = tolower(str[i]);
	lower[i] = 0;
}

void str_toupper(const char *str, char *upper, size_t *upper_len) {
	size_t str_len = strlen(str);
	//Avoid overflow
	ASSERT(*upper_len >= str_len);

	//Update lower len
	*upper_len = str_len;

	size_t i = 0;
	for(; i < str_len; i++) upper[i] = toupper(str[i]);
	upper[i] = 0;
}

// Utility function to increase the size of a buffer.
void str_ExtendBuffer(
	char **buf,           // buffer to populate
	size_t *bufferLen,    // size of buffer
	size_t extensionLen   // number of bytes to add
) {
	*bufferLen += extensionLen;
	*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
}


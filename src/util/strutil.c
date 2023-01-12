/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <string.h>
#include <ctype.h>
#include "RG.h"
#include "rmalloc.h"
#include "utf8proc/utf8proc.h"

// determine the length of the string
// not in terms of the number of bytes in the string
// but in terms of the number of Unicode characters in the string
int str_length
(
    const char *str
) {
    utf8proc_int32_t c; // hold current Unicode character

    size_t i       = 0;
    int    len     = 0;
    size_t str_len = strlen(str);  // length of the string in bytes

    // while the current position is less than length in bytes
    while(i < str_len) {
        // increment current position by number of bytes in Unicode character
        i += utf8proc_iterate((const utf8proc_uint8_t *)(str + i), -1, &c);
        // increment length of the string in terms of Unicode characters
        len++;
    }

    // return the length of the string in terms of Unicode characters
    return len;
}

void str_tolower
(
	const char *str,
	char *lower,
	size_t *lower_len
) {
	size_t str_len = strlen(str);
	//Avoid overflow
	ASSERT(*lower_len >= str_len);

	//Update lower len
	*lower_len = str_len;

	utf8proc_int32_t c;
	size_t i = 0;
	int w = 0;
	while(i < str_len) {
		w = utf8proc_iterate((const utf8proc_uint8_t *)(str + i), -1, &c);
		utf8proc_encode_char(utf8proc_tolower(c), (utf8proc_uint8_t *)(lower + i));
		i+=w;
	}
	lower[i] = 0;
}

void str_toupper
(
	const char *str,
	char *upper,
	size_t *upper_len
) {
	size_t str_len = strlen(str);
	//Avoid overflow
	ASSERT(*upper_len >= str_len);

	//Update lower len
	*upper_len = str_len;

	utf8proc_int32_t c;
	size_t i = 0;
	int w = 0;
	while(i < str_len) {
		w = utf8proc_iterate((const utf8proc_uint8_t *)(str + i), -1, &c);
		utf8proc_encode_char(utf8proc_toupper(c), (utf8proc_uint8_t *)(upper + i));
		i+=w;
	}
	upper[i] = 0;
}

// Utility function to increase the size of a buffer.
void str_ExtendBuffer
(
	char **buf,           // buffer to populate
	size_t *bufferLen,    // size of buffer
	size_t extensionLen   // number of bytes to add
) {
	*bufferLen += extensionLen;
	*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
}

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

void str_tolower_ascii
(
	const char *str,
	char *lower,
	size_t *lower_len
) {
 	size_t str_len = strlen(str);
 	// avoid overflow
 	ASSERT(*lower_len >= str_len);

 	// update lower len
 	*lower_len = str_len;

 	for(size_t i = 0; i < str_len; i++) lower[i] = tolower(str[i]);
 	lower[str_len] = 0;
 }

// determine the length of the string
// not in terms of the number of bytes in the string
// but in terms of the number of Unicode characters in the string
int str_length
(
    const char *str
) {
	// hold current Unicode character
    utf8proc_int32_t c;

    int len = 0;

    // while we didn't get to the end of the string
    while(str[0] != 0) {
        // increment current position by number of bytes in Unicode character
        str += utf8proc_iterate((const utf8proc_uint8_t *)str, -1, &c);
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
	// avoid overflow
	ASSERT(*lower_len >= str_len);

	// update lower len
	*lower_len = str_len;

	// hold current Unicode character
	utf8proc_int32_t c;
	// while we didn't get to the end of the string
	while(str[0] != 0) {
		// increment current position by number of bytes in Unicode character
		str   += utf8proc_iterate((const utf8proc_uint8_t *)str, -1, &c);
		// write the Unicode character to the buffer
		lower += utf8proc_encode_char(utf8proc_tolower(c), (utf8proc_uint8_t *)lower);
	}
	lower[0] = 0;
}

void str_toupper
(
	const char *str,
	char *upper,
	size_t *upper_len
) {
	size_t str_len = strlen(str);
	// avoid overflow
	ASSERT(*upper_len >= str_len);

	// update lower len
	*upper_len = str_len;

	// hold current Unicode character
	utf8proc_int32_t c;
	// while we didn't get to the end of the strings
	while(str[0] != 0) {
		// increment current position by number of bytes in Unicode character
		str   += utf8proc_iterate((const utf8proc_uint8_t *)str, -1, &c);
		// write the Unicode character to the buffer
		upper += utf8proc_encode_char(utf8proc_toupper(c), (utf8proc_uint8_t *)upper);
	}
	upper[0] = 0;
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

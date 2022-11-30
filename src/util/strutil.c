/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <string.h>
#include <ctype.h>
#include "RG.h"
#include "rmalloc.h"
#include "../../deps/icu/build/include/unicode/ustring.h"

static void str_changecase(const char *str, char *result, size_t *result_len, const char *locale, bool toUpper) {
	size_t str_len = strlen(str);
	//Avoid overflow
	assert(*result_len >= str_len);
	//Update result len
	*result_len = str_len;

	size_t u_og_len = str_len;
	size_t u_result_len = str_len;

	UChar* original = (UChar *) rm_malloc((u_og_len + 1) * sizeof(UChar));
    UChar* converted = (UChar *) rm_malloc((u_result_len + 1) * sizeof(UChar));
    UErrorCode errorCode = U_ZERO_ERROR;

    u_uastrcpy(original, str);

    size_t size;
	if (toUpper) {
		size = u_strToUpper(converted, u_result_len, original, u_og_len, locale, &errorCode);
	} else {
		size = u_strToLower(converted, u_result_len, original, u_og_len, locale, &errorCode);
	}

    // if the result is larger than allocated length
    if (size > u_result_len) {
        u_result_len = size;
		*result_len = size;
        if (toUpper) {
			u_strToUpper(converted, u_result_len, original, u_og_len, locale, &errorCode);
		} else {
			u_strToLower(converted, u_result_len, original, u_og_len, locale, &errorCode);
		}
    }
	if (U_FAILURE(errorCode)) {
		// do something to handle error
		*result = *str;
	} else {
		u_austrcpy(result, converted);
	}
	rm_free(original);
	rm_free(converted);
}

void str_tolower(const char *str, char *lower, size_t *lower_len, const char *locale) {
	str_changecase(str, lower, lower_len, locale, false);
}

void str_toupper(const char *str, char *upper, size_t *upper_len, const char *locale) {
	str_changecase(str, upper, upper_len, locale, true);
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


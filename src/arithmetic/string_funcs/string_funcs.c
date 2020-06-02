/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "string_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../util/uuid.h"
#include <ctype.h>
#include <assert.h>

static inline void _toLower(const char *str, char *lower, size_t lower_len) {
	size_t i = 0;
	for(; i < lower_len; i++) lower[i] = tolower(str[i]);
	lower[i] = 0;
}

static inline void _toUpper(const char *str, char *upper, size_t upper_len) {
	size_t i = 0;
	for(; i < upper_len; i++) upper[i] = toupper(str[i]);
	upper[i] = 0;
}

/* returns a string containing the specified number of leftmost characters of the original string. */
SIValue AR_LEFT(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	int64_t newlen = argv[1].longval;
	if(strlen(argv[0].stringval) <= newlen) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(argv[0].stringval);
	}
	char *left_str = rm_malloc((newlen + 1) * sizeof(char));
	strncpy(left_str, argv[0].stringval, newlen * sizeof(char));
	left_str[newlen] = '\0';
	return SI_TransferStringVal(left_str);
}

/* returns the original string with leading whitespace removed. */
SIValue AR_LTRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	char *trimmed = argv[0].stringval;

	while(*trimmed == ' ') {
		trimmed ++;
	}

	return SI_DuplicateStringVal(trimmed);
}

/* returns a string containing the specified number of rightmost characters of the original string. */
SIValue AR_RIGHT(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	int64_t newlen = argv[1].longval;
	int64_t start = strlen(argv[0].stringval) - newlen;

	if(start <= 0) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(argv[0].stringval);
	}
	return SI_DuplicateStringVal(argv[0].stringval + start);
}

/* returns the original string with trailing whitespace removed. */
SIValue AR_RTRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	char *str = argv[0].stringval;

	size_t i = strlen(str);
	while(i > 0 && str[i - 1] == ' ') {
		i --;
	}

	char *trimmed = rm_malloc((i + 1) * sizeof(char));
	strncpy(trimmed, str, i);
	trimmed[i] = '\0';

	return SI_TransferStringVal(trimmed);
}

/* returns a string in which the order of all characters in the original string have been reversed. */
SIValue AR_REVERSE(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *str = argv[0].stringval;
	size_t str_len = strlen(str);
	char *reverse = rm_malloc((str_len + 1) * sizeof(char));

	int i = str_len - 1;
	int j = 0;
	while(i >= 0) {
		reverse[j++] = str[i--];
	}
	reverse[j] = '\0';
	return SI_TransferStringVal(reverse);
}

/* returns a substring of the original string, beginning with a 0-based index start and length. */
SIValue AR_SUBSTRING(SIValue *argv, int argc) {
	/*
	    argv[0] - original string
	    argv[1] - start position
	    argv[2] - length
	    If length is omitted, the function returns the substring starting at the position given by start and extending to the end of original.
	    If either start or length is null or a negative integer, an error is raised.
	    If start is 0, the substring will start at the beginning of original.
	    If length is 0, the empty string will be returned.
	*/
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	char *original = argv[0].stringval;
	int64_t original_len = strlen(original);
	int64_t start = argv[1].longval;
	int64_t length;

	/* Make sure start doesn't overreach. */
	assert(start < original_len && start >= 0);

	if(argc == 2) {
		length = original_len - start;
	} else {
		length = argv[2].longval;
		assert(length >= 0);

		/* Make sure length does not overreach. */
		if(start + length > original_len) {
			length = original_len - start;
		}
	}

	char *substring = rm_malloc((length + 1) * sizeof(char));
	strncpy(substring, original + start, length);
	substring[length] = '\0';

	return SI_TransferStringVal(substring);
}

/* returns the original string in lowercase. */
SIValue AR_TOLOWER(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	short lower_len = strlen(original);
	char *lower = rm_malloc((lower_len + 1) * sizeof(char));
	_toLower(original, lower, lower_len);
	return SI_TransferStringVal(lower);
}

/* returns the original string in uppercase. */
SIValue AR_TOUPPER(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	size_t upper_len = strlen(original);
	char *upper = rm_malloc((upper_len + 1) * sizeof(char));
	_toUpper(original, upper, upper_len);
	return SI_TransferStringVal(upper);
}

/* converts an integer, float or boolean value to a string. */
SIValue AR_TOSTRING(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	size_t len = SIValue_StringJoinLen(argv, 1, "");
	char *str = rm_malloc(len * sizeof(char));
	size_t bytesWritten = 0;
	SIValue_ToString(argv[0], &str, &len, &bytesWritten);
	return SI_TransferStringVal(str);
}

/* returns the original string with leading and trailing whitespace removed. */
SIValue AR_TRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	SIValue ltrim = AR_LTRIM(argv, argc);
	SIValue trimmed = AR_RTRIM(&ltrim, 1);
	return trimmed;
}

/* returns true if argv[1] is a substring of argv[0]. */
SIValue AR_CONTAINS(SIValue *argv, int argc) {
	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	const char *hay = argv[0].stringval;
	const char *needle = argv[1].stringval;

	// See if needle is in hay.
	bool found = (strstr(hay, needle) != NULL);
	return SI_BoolVal(found);
}

/* returns true if argv[0] starts with argv[1]. */
SIValue AR_STARTSWITH(SIValue *argv, int argc) {
	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	const char *str = argv[0].stringval;
	const char *sub_string = argv[1].stringval;
	size_t str_len = strlen(str);
	size_t sub_string_len = strlen(sub_string);

	// If sub-string is longer then string return quickly.
	if(sub_string_len > str_len) return SI_BoolVal(false);

	// Compare character by character, see if there's a match.
	for(int i = 0; i < sub_string_len; i++) {
		if(str[i] != sub_string[i]) return SI_BoolVal(false);
	}

	return SI_BoolVal(true);
}

/* returns true if argv[0] ends with argv[1]. */
SIValue AR_ENDSWITH(SIValue *argv, int argc) {
	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	const char *str = argv[0].stringval;
	const char *sub_string = argv[1].stringval;
	size_t str_len = strlen(str);
	size_t sub_string_len = strlen(sub_string);

	// If sub-string is longer then string return quickly.
	if(sub_string_len > str_len) return SI_BoolVal(false);

	// Advance str to the "end"
	str += (str_len - sub_string_len);
	// Compare character by character, see if there's a match.
	for(int i = 0; i < sub_string_len; i++) {
		if(str[i] != sub_string[i]) return SI_BoolVal(false);
	}

	return SI_BoolVal(true);
}

//==============================================================================
//=== Scalar functions =========================================================
//==============================================================================

SIValue AR_RANDOMUUID(SIValue *argv, int argc) {
	char *uuid = UUID_New();
	return SI_TransferStringVal(uuid);
}

void Register_StringFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, T_INT64);
	func_desc = AR_FuncDescNew("left", AR_LEFT, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("ltrim", AR_LTRIM, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, T_INT64);
	func_desc = AR_FuncDescNew("right", AR_RIGHT, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("rtrim", AR_RTRIM, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("reverse", AR_REVERSE, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, T_INT64);
	types = array_append(types, T_INT64);
	func_desc = AR_FuncDescNew("substring", AR_SUBSTRING, 2, 3, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("tolower", AR_TOLOWER, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("toupper", AR_TOUPPER, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("tostring", AR_TOSTRING, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("trim", AR_TRIM, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("contains", AR_CONTAINS, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("starts with", AR_STARTSWITH, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (T_STRING | T_NULL));
	types = array_append(types, (T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("ends with", AR_ENDSWITH, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("randomuuid", AR_RANDOMUUID, 0, 0, types, false);
	AR_RegFunc(func_desc);
}

/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "string_funcs.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../util/uuid.h"
#include "utf8proc/utf8proc.h"
#include "../../util/rmalloc.h"
#include "../../util/strutil.h"
#include "../../datatypes/map.h"
#include "../../datatypes/array.h"
#include "../../util/json_encoder.h"
#include "../deps/oniguruma/src/oniguruma.h"
#include <math.h>

// toString supports only integer, float, string, boolean, point, duration, 
// date, time, localtime, localdatetime or datetime values
#define STRINGABLE (SI_NUMERIC | T_POINT | T_DURATION | T_DATETIME | T_STRING | T_BOOL)

// returns a string containing the specified number of leftmost characters of the original string.
SIValue AR_LEFT(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	
	int64_t newlen = -1;
	if(SI_TYPE(argv[1]) == T_INT64) {
		newlen = argv[1].longval;
	} 
	if(newlen < 0) {
		ErrorCtx_SetError("length must be a non-negative integer");
		return SI_NullVal();
	}

	if(strlen(argv[0].stringval) <= newlen) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(argv[0].stringval);
	}

	// determine new string byte size
	utf8proc_int32_t c;
	int64_t newlen_bytes = 0;
	const char *str = argv[0].stringval;
	for (int i = 0; i < newlen; i++) {
		newlen_bytes += utf8proc_iterate((const utf8proc_uint8_t *)(str+newlen_bytes), -1, &c);
	}

	char *left_str = rm_malloc((newlen_bytes + 1) * sizeof(char));
 	strncpy(left_str, str, newlen_bytes * sizeof(char));
 	left_str[newlen_bytes] = '\0';

	return SI_TransferStringVal(left_str);
}

// returns the original string with leading whitespace removed.
SIValue AR_LTRIM(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	char *trimmed = argv[0].stringval;

	while(*trimmed == ' ') {
		trimmed ++;
	}

	return SI_DuplicateStringVal(trimmed);
}

// returns a string containing the specified number of rightmost characters of the original string.
SIValue AR_RIGHT(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	
	int64_t newlen = -1;
	if(SI_TYPE(argv[1]) == T_INT64) {
		newlen = argv[1].longval;
	}
	if(newlen < 0) {
		ErrorCtx_SetError("length must be a non-negative integer");
		return SI_NullVal();
	}

	const char *str = argv[0].stringval;
	int64_t start   = str_length(str) - newlen;

	if(start <= 0) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(str);
	}

	utf8proc_int32_t c;
	int64_t start_bytes = 0;
	for (int i = 0; i < start; i++) {
		start_bytes += utf8proc_iterate((const utf8proc_uint8_t *)(str+start_bytes), -1, &c);
	}

	return SI_DuplicateStringVal(str + start_bytes);
}

// returns the original string with trailing whitespace removed.
SIValue AR_RTRIM(SIValue *argv, int argc, void *private_data) {
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

// incase the parameter type is 
// 1. string - returns a string in which the order of all characters in the original string have been reversed.
// 2. array  - returns an array in which the order of all elements in the original array have been reversed.
SIValue AR_REVERSE(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	SIValue value = argv[0];
	if(SI_TYPE(value) == T_STRING) {
		// string reverse
		char *str = value.stringval;
		size_t str_len = strlen(str);
		char *reverse = rm_malloc((str_len + 1) * sizeof(char));

		char *reverse_i = reverse + str_len;
		utf8proc_int32_t c;
		utf8proc_ssize_t w;
		while(str[0] != 0) {
			w = utf8proc_iterate((const utf8proc_uint8_t *)str, -1, &c);
			str += w;
			reverse_i -= w;
			utf8proc_encode_char(c, (utf8proc_uint8_t *)reverse_i);
		}
		reverse[str_len] = '\0';
		return SI_TransferStringVal(reverse);
	} else {
		SIValue reverse = SI_CloneValue(value);
		array_reverse(reverse.array);
		return reverse;
	}
}

// returns a substring of the original string, beginning with a 0-based index start and length.
SIValue AR_SUBSTRING(SIValue *argv, int argc, void *private_data) {
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

	int64_t length;
	const char   *original     = argv[0].stringval;
	const int64_t original_len = strlen(original);
	const int64_t start        = argv[1].longval;

	/* Make sure start doesn't overreach. */
	if(start < 0) {
		ErrorCtx_SetError("start must be a non-negative integer");
		return SI_NullVal();
	}

	if(start >= original_len) {
		return SI_ConstStringVal("");
	}

	const int64_t suffix_len = original_len - start;
	if(argc == 2) {
		length = suffix_len;
	} else {
		length = argv[2].longval;
		if(length < 0) {
			ErrorCtx_SetError("length must be a non-negative integer");
			return SI_ConstStringVal("");
		}

		/* Make sure length does not overreach. */
		length = MIN(length, suffix_len);
	}

	utf8proc_int32_t c;
	// find the start position to copy from
	const char *start_p = original;
	for (int i = 0; i < start; i++) {
		start_p += utf8proc_iterate((const utf8proc_uint8_t *)start_p, -1, &c);
	}

	// find the end position
	const char *end_p = start_p;
	for (int i = 0; i < length; i++) {
		end_p += utf8proc_iterate((const utf8proc_uint8_t *)end_p, -1, &c);
	}

	int len = end_p - start_p;
	char *substring = rm_malloc((len + 1) * sizeof(char));
	strncpy(substring, start_p, len);
	substring[len] = '\0';

	return SI_TransferStringVal(substring);
}

// given a list of strings and an optional delimiter
// return a concatenation of all the strings using the given delimiter
// string.join(list, delimiter = '') -> string
SIValue AR_JOIN(SIValue *argv, int argc, void *private_data) {
	SIValue list = argv[0];
	if(SI_TYPE(list) == T_NULL) {
		return SI_NullVal();
	}

	char *delimiter = "";
	if(argc == 2) {
		delimiter = argv[1].stringval;
	}

	uint32_t count = SIArray_Length(list);

	size_t delimeter_len = strlen(delimiter);
	uint str_len = delimeter_len * (count - 1);
	for(uint i = 0; i < count; i++) {
		SIValue str = SIArray_Get(list, i);
		if(SI_TYPE(str) != T_STRING) {
			// all elements in the list should be string.
			Error_SITypeMismatch(str, T_STRING);
			return SI_NullVal();
		}

		str_len += strlen(str.stringval);
	}

	int cur_len = 0;
	char *res = rm_malloc(str_len + 1);
	for(uint i = 0; i < count - 1; i++) {
		SIValue str = SIArray_Get(list, i);
		memcpy(res + cur_len, str.stringval, strlen(str.stringval));
		cur_len += strlen(str.stringval);
		memcpy(res + cur_len, delimiter, delimeter_len);
		cur_len += delimeter_len;
	}
	SIValue str = SIArray_Get(list, count - 1);
	memcpy(res + cur_len, str.stringval, strlen(str.stringval));
	res[str_len] = '\0';

	return SI_TransferStringVal(res);
}

typedef struct {
	SIValue *list;
	const char *str;
} match_regex_scan_cb_args;

static int match_regex_scan_cb(int n, int pos, OnigRegion *region, void *arg) {
	match_regex_scan_cb_args *args = (match_regex_scan_cb_args *)arg;
	SIValue *list = args->list;
	const char *str = args->str;
	SIValue subList = SIArray_New(region->num_regs);
	assert(region->num_regs > 0);

	for (int i = 0; i < region->num_regs; i++) {
		int substr_len = region->end[i] - region->beg[i];
		char *substr = rm_strndup(str + region->beg[i], substr_len);
		SIArray_Append(&subList, SI_TransferStringVal(substr));
		rm_free(substr);
	}

	SIArray_Append(list, subList);
	SIValue_Free(subList);
	return 0;
}

// given a string and a regular expression,
// return an array of all matches and matching regions
// string.matchRegEx(str, regex) -> array(array(string))
SIValue AR_MATCHREGEX(SIValue *argv, int argc, void *private_data) {
	SIValue list = SIArray_New(0);
	if(SI_TYPE(argv[0]) == T_NULL || SI_TYPE(argv[1]) == T_NULL) {
		return list;
	}

	regex_t *regex;
	OnigErrorInfo einfo;
	OnigRegion *region    = onig_region_new();
	const char *str       = argv[0].stringval;
	const char *regex_str = argv[1].stringval;

	int rv = onig_new(&regex, (const UChar *)regex_str, 
		(const UChar *)(regex_str + strlen(regex_str)), ONIG_OPTION_DEFAULT,
		ONIG_ENCODING_UTF8, ONIG_SYNTAX_JAVA, &einfo);
	if(rv != ONIG_NORMAL) {
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str((UChar* )s, rv, &einfo);
		ErrorCtx_SetError("Invalid regex, err=%s", s);
		onig_free(regex);
		onig_region_free(region, 1);
		SIValue_Free(list);
		return SI_NullVal();
	}

	match_regex_scan_cb_args args = {
		.list = &list,
		.str = str
	};

	rv = onig_scan(regex, (const UChar *)str,
		(const UChar *)(str + strlen(str)), region, ONIG_OPTION_DEFAULT,
		match_regex_scan_cb, &args);
	if(rv < 0) {
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str((OnigUChar* )s, rv);
		ErrorCtx_SetError("Invalid regex, err=%s", s);
		onig_free(regex);
		onig_region_free(region, 1);
		SIValue_Free(list);
		return SI_NullVal();
	}

	onig_free(regex);
	onig_region_free(region, 1);

	return list;
}

typedef struct {
	char *res;
	uint32_t res_len;
	const char *str;
	uint32_t str_ind; // current index in str
	const char *replacement;
	uint32_t replacement_len;
} replace_regex_scan_cb_args;

static int replace_regex_scan_cb(int n, int pos, OnigRegion *region, void *arg) {
	replace_regex_scan_cb_args *args = (replace_regex_scan_cb_args *)arg;
	const char *str = args->str;
	assert(region->num_regs > 0);

	// reallocate new str size
	int str_copy_len = region->beg[0] - args->str_ind;
	int str_size = args->res_len + str_copy_len + args->replacement_len + 1;
	args->res = rm_realloc(args->res, str_size);

	// copy the string between the last match and the current match
	memcpy(args->res + args->res_len, str + args->str_ind, str_copy_len);
	args->str_ind = region->end[0];
	args->res_len += str_copy_len;

	// copy the replacement string
	memcpy(args->res + args->res_len, args->replacement, args->replacement_len);
	args->res_len += args->replacement_len;

	args->res[args->res_len] = '\0';

	return 0;
}

// given a string and a regular expression,
// return a string after replacing each regex match with a given replacement.
// string.replaceRegEx(str, regex, replacement) -> string
SIValue AR_REPLACEREGEX(SIValue *argv, int argc, void *private_data) {
	if(SI_TYPE(argv[0]) == T_NULL || SI_TYPE(argv[1]) == T_NULL) {
		return SI_NullVal();
	}

	char       *replacement = "";
	const char *str         = argv[0].stringval;
	const char *regex_str   = argv[1].stringval;

	if(argc == 3) {
		if(SI_TYPE(argv[2]) == T_NULL) {
			return SI_NullVal();
		}

		replacement = argv[2].stringval;
	}

	regex_t *regex;
	OnigErrorInfo einfo;
	OnigRegion *region = onig_region_new();

	int rv = onig_new(&regex, (const UChar *)regex_str, 
		(const UChar *)(regex_str + strlen(regex_str)), ONIG_OPTION_DEFAULT,
		ONIG_ENCODING_UTF8, ONIG_SYNTAX_JAVA, &einfo);
	if(rv != ONIG_NORMAL) {
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str((UChar* )s, rv, &einfo);
		ErrorCtx_SetError("Invalid regex, err=%s", s);
		onig_free(regex);
		onig_region_free(region, 1);
		return SI_NullVal();
	}

	replace_regex_scan_cb_args args = {
		.res = NULL,
		.res_len = 0,
		.str = str,
		.str_ind = 0,
		.replacement = replacement,
		.replacement_len = strlen(replacement)
	};

	rv = onig_scan(regex, (const UChar *)str,
		(const UChar *)(str + strlen(str)), region, ONIG_OPTION_DEFAULT,
		replace_regex_scan_cb, &args);
	if(rv < 0) {
		char s[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str((OnigUChar* )s, rv);
		ErrorCtx_SetError("Invalid regex, err=%s", s);
		onig_free(regex);
		onig_region_free(region, 1);
		return SI_NullVal();
	}

	onig_free(regex);
	onig_region_free(region, 1);

	// copy the remaining string
	int str_copy_len = strlen(str) - args.str_ind;
	args.res = rm_realloc(args.res, (args.res_len + str_copy_len + 1)*sizeof(char));
	memcpy(args.res + args.res_len, str + args.str_ind, str_copy_len*sizeof(char));
	args.res[args.res_len + str_copy_len] = '\0';

	return SI_TransferStringVal(args.res);
}

// returns the original string in lowercase.
SIValue AR_TOLOWER(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	char *lower = str_tolower(original);
	return SI_TransferStringVal(lower);
}

// returns the original string in uppercase.
SIValue AR_TOUPPER(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	char *upper = str_toupper(original);
	return SI_TransferStringVal(upper);
}

// converts an integer, float or boolean value to a string.
SIValue AR_TOSTRING(SIValue *argv, int argc, void *private_data) {
	if(SI_TYPE(argv[0]) & STRINGABLE) {
		size_t len = SIValue_StringJoinLen(argv, 1, "");
		char *str = rm_malloc(len * sizeof(char));
		size_t bytesWritten = 0;
		SIValue_ToString(argv[0], &str, &len, &bytesWritten);
		return SI_TransferStringVal(str);
	}
	else {
		return SI_NullVal();
	}
}

// Returns a JSON string representation of a map value.
SIValue AR_TOJSON(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *buf = JsonEncoder_SIValue(argv[0]);
	return SI_TransferStringVal(buf);
}

// returns the original string with leading and trailing whitespace removed.
SIValue AR_TRIM(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	SIValue ltrim = AR_LTRIM(argv, argc, NULL);
	SIValue trimmed = AR_RTRIM(&ltrim, 1, NULL);
	SIValue_Free(ltrim);
	return trimmed;
}

// returns true if argv[1] is a substring of argv[0].
SIValue AR_CONTAINS(SIValue *argv, int argc, void *private_data) {
	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	const char *hay = argv[0].stringval;
	const char *needle = argv[1].stringval;

	// See if needle is in hay.
	bool found = (strstr(hay, needle) != NULL);
	return SI_BoolVal(found);
}

// returns true if argv[0] starts with argv[1].
SIValue AR_STARTSWITH(SIValue *argv, int argc, void *private_data) {
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

// returns true if argv[0] ends with argv[1].
SIValue AR_ENDSWITH(SIValue *argv, int argc, void *private_data) {
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

// returns a string in which all occurrences of a specified string in the original string have been replaced by ANOTHER (specified) string.
// for example: RETURN replace('Well I wish I was in the land of cotton', 'cotton', 'the free')
// the result is Well I wish I was in the land of the free
SIValue AR_REPLACE(SIValue *argv, int argc, void *private_data) {
	// No string contains null.
	if(SIValue_IsNull(argv[0]) ||
	   SIValue_IsNull(argv[1]) ||
	   SIValue_IsNull(argv[2])) return SI_NullVal();

	// argv[0] is the original string to be manipulated
	// argv[1] is the search sub string to be replaced
	// argv[2] is the string to be replaced with
	const char *str            =  argv[0].stringval;
	const char *old_string     =  argv[1].stringval;
	const char *new_string     =  argv[2].stringval;
	size_t      str_len        =  strlen(str);
	size_t      old_string_len =  strlen(old_string);
	size_t      new_string_len =  strlen(new_string);

	const char *ptr  = str;
	const char **arr = array_new(const char *, 0);

	// if any parameter is not a valid utf8 string return the original string
	if(!str_utf8_validate(old_string) || !str_utf8_validate(new_string) ||
		!str_utf8_validate(str)) {
		return SI_DuplicateStringVal(str);
	}

	while(ptr <= str + str_len) {
		// find pointer to next substring
		ptr = strstr(ptr, old_string);

		// if no substring found, then break from the loop
		if(ptr == NULL) break;

		// store ptr for replace use
		array_append(arr, ptr);

		// increment our string pointer in case search string is empty move one char
		ptr += old_string_len == 0 ? 1 : old_string_len;
	}

	int occurrences = array_len(arr);

	// if sub string not found return original string
	if(occurrences == 0) {
		array_free(arr);
		return SI_DuplicateStringVal(str);
	}

	// calculate new buffer size
	size_t buffer_size = strlen(str) + (occurrences * new_string_len) - (occurrences * old_string_len);

	// allocate buffer
	char *buffer = (char*) rm_malloc(sizeof(char) * buffer_size + 1);

	// set pointers to start point
	ptr = str;
	char *buffer_ptr = buffer;

	// iterate occurrences
	for (int i = 0; i < occurrences; i++) {
		// calculate len to copy from last to current occurance
		int len = arr[i] - ptr;

		// copy part from original string
		strncpy(buffer_ptr, ptr, len);

		// move forward to copy more data to the buffer
		buffer_ptr += len;

		// copy new string instead of old string
		strcpy(buffer_ptr, new_string);

		// move forward to copy more data to the buffer
		buffer_ptr += new_string_len;

		// move forwart to copy more data from the original string
		ptr = arr[i] + old_string_len;
	}

	// copy rest of the string from the original string
	strcpy(buffer_ptr, ptr);

	buffer[buffer_size] = '\0';

	array_free(arr);

	return SI_TransferStringVal(buffer);
}

// returns a list of strings resulting from the splitting of the original string around matches of the given delimiter
SIValue AR_SPLIT(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) {
		return SI_NullVal();
	}

	char       *str           = argv[0].stringval;
	const char *delimiter     = argv[1].stringval;
	size_t      str_len       = strlen(str);
	size_t      delimiter_len = strlen(delimiter);
	SIValue     tokens        = SIArray_New(1);

	if(delimiter_len == 0) {
		if(strlen(str) == 0) {
			SIArray_Append(&tokens, SI_ConstStringVal(""));
		} else {
			utf8proc_int32_t c;
			utf8proc_uint8_t token[5];
			const utf8proc_uint8_t *str_i = (const utf8proc_uint8_t *)str;
			while(str_i[0] != 0) {
				str_i += utf8proc_iterate(str_i, -1, &c);
				int i  = utf8proc_encode_char(utf8proc_tolower(c), token);
				token[i] = '\0';
				SIArray_Append(&tokens, SI_ConstStringVal((const char *)token));
			}
		}
	} else if(str_len == 0) {
		SIArray_Append(&tokens, SI_ConstStringVal(""));
	} else {
		size_t rest_len   = str_len;
		const char *start = str;
		bool delimiter_found = false;
		while(rest_len >= delimiter_len) {
			// find bytes length from start to delimiter
			int len = 0;
			delimiter_found = false;
			while(len <= rest_len - delimiter_len) {
				if(strncmp(start + len, delimiter, delimiter_len) == 0) {
					delimiter_found = true;
					break;
				}
				len++;
			}
			if(!delimiter_found) {
				break;
			}
			SIValue si_token = SI_TransferStringVal(rm_strndup(start, len));
			SIArray_Append(&tokens, si_token);
			SIValue_Free(si_token);
			start += len + delimiter_len;
			rest_len -= len + delimiter_len;
		}
		if(rest_len > 0 || delimiter_found) {
			SIValue si_token = SI_ConstStringVal(start);
			SIArray_Append(&tokens, si_token);
		}
	}

	return tokens;
}

#define __SWAP(T, a, b) do { T tmp = a; a = b; b = tmp; } while (0)

/*

Example of the flood filled matrix:
			S2
	+---+---+---+---+---+---+---+
	|   |   | b | 2 | 1 | 2 | 3 |
	+---+---+---+---+---+---+---+
	|   | 0 | 1 | 2 | 3 | 4 | 5 |
	+---+---+---+---+---+---+---+
S1	| d | 1 | 1 | 2 | 3 | 4 | 5 |
	+---+---+---+---+---+---+---+
	| 1 | 2 | 2 | 2 | 2 | 3 | 4 |
	+---+---+---+---+---+---+---+
	| 2 | 3 | 3 | 2 | 3 | 2 | 3 |
	+---+---+---+---+---+---+---+
	| 3 | 4 | 4 | 3 | 3 | 3 | 2 |
	+---+---+---+---+---+---+---+


Example of the flood filled matrix:

substitutionWeight = 5
deletionWeight = 1
insertionWeight = 5

			S2
	+---+---+---+---+
	|   |   | a | b |
	+---+---+---+---+
	|   | 0 | 1 | 2 |
	+---+---+---+---+
S1	| a | 1 | 0 | 3 |
	+---+---+---+---+

			S2
	+---+---+---+
	|   |   | a |
	+---+---+---+
	|   | 0 | 1 |
	+---+---+---+
S1	| a | 1 | 0 |
	+---+---+---+
	| b | 2 | 1 |
	+---+---+---+

implementation of the Levenshtein distance algorithm
using Wagner–Fischer algorithm
Wagner–Fischer algorithm is comutative iff insertion and deletion have the same weights
as you can see from the example above. */
static double levenshtein_distance(
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2,
	double insertionWeight,
	double deletionWeight,
	double substitutionWeight
) {

	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	// dynamically allocate the array since it might be large
	double *distance_array = (double *)rm_malloc((len2+1)*sizeof(double));
	distance_array[0] = 0;

	// initialize the first row
	for(int i = 1; i <= len2; i++) {
		distance_array[i] = distance_array[i-1] + insertionWeight;
	}

	// build the matrix - always keep the last row calculated in the matrix
	double upper_left;
	for(int i = 1; i <= len1; i++) {
		// store upper_left cause it will be overwritten
		upper_left = distance_array[0];
		distance_array[0] = upper_left + deletionWeight;

		for(int j = 1; j <= len2; j++) {
			double substitutionCost = (S2[j-1] == S1[i-1]) ? 0 : substitutionWeight;
			double min = fmin(
				fmin(distance_array[j] + deletionWeight,    /* deletion from S1 */
					distance_array[j-1] + insertionWeight), /* insertion to S1 */
					upper_left + substitutionCost           /* substitution */
				);
			upper_left = distance_array[j];
			distance_array[j] = min;
		}
	}

	double res = distance_array[len2];
	rm_free(distance_array);
	rm_free(S1);
	rm_free(S2);
	return res;
}


// implementation of the Damerau Levenshtein distance algorithm
// (Optimal string alignment distance)
// the algorithm is comutative iff insertion and deletion have same weights
static double damerau_levenshtein_distance_osa (
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2,
	double insertionWeight,
	double deletionWeight,
	double substitutionWeight,
	double transpositionWeight
) {

	int r,c;
	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	// dynamically allocate the array since it might be large
	double **distance_array = rm_malloc(3 * sizeof(double *));
	for(r = 0; r < 3; r++) {
		distance_array[r] = rm_malloc((len2 + 1) * sizeof(double));
	}

	// initialize the first item in the matrix
	distance_array[0][0] = 0;

	// initialize the first row
	for(c = 1; c <= len2; c++) {
		distance_array[0][c] = distance_array[0][c-1] + insertionWeight;
	}

	// build the matrix - always keep the 3 last rows calculated in the matrix
	int prev_row = 0;
	for(int i = 1, r = 1; i <= len1; i++, prev_row = r, r = i%3) {
		// initialize the first element in the row
		distance_array[r][0] = distance_array[prev_row][0] + deletionWeight;

		// calculate the rest of the row
		for(int j = 1; j <= len2; j++) {
			double substitutionCost = (S2[j-1] == S1[i-1]) ? 0 : substitutionWeight;
			double min = fmin(
				fmin(distance_array[prev_row][j] + deletionWeight,      /* deletion from S1 */
					distance_array[r][j-1] + insertionWeight),          /* insertion to S1 */
					distance_array[prev_row][j-1] + substitutionCost    /* substitution */
				);
			if(i > 1 && j > 1 && S1[i-1] == S2[j-2] && S1[i-2] == S2[j-1]) {
				min = fmin(
					min,
					distance_array[(i-2)%3][j-2] + transpositionWeight  /* transposition */
					);
			}
			distance_array[r][j] = min;
		}
	}

	double res = distance_array[prev_row][len2];
	for(r = 0; r < 3; r++) {
		rm_free(distance_array[r]);
	}
	rm_free(distance_array);
	rm_free(S1);
	rm_free(S2);
	return res;
}

// implementation of the Damerau Levenshtein distance algorithm
// the algorithm is comutative iff insertion and deletion have same weights
// see good (but partially wrong) explanation here: https://www.lemoda.net/text-fuzzy/damerau-levenshtein/
// and here: https://en.wikipedia.org/wiki/Levenshtein_distance
static double damerau_levenshtein_distance (
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2,
	double insertionWeight,
	double deletionWeight,
	double substitutionWeight,
	double transpositionWeight
) {

	int r,c;
	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	// dynamically allocate the array since it might be large
	double **distance_array = rm_malloc((len1 + 2) * sizeof(double *));
	for(r = 0; r < len1 + 2; r++) {
		distance_array[r] = rm_malloc((len2 + 2) * sizeof(double));
	}

	// dynamically allocate the array since it might be large
	// last_match_row[i] stores the last row in distance_array that holds the character S2[i]
	// initialize last_match_row to point to the 2nd row
	// so the previous one points to the infinity row
	int *last_match_row = rm_malloc((len2+2) * sizeof(int));
	for(r = 0; r < len2 + 2; r++) {
		last_match_row[r] = 1;
	}

	// initialize the first two items in
	// the first 2 rows
	distance_array[0][0] = INFINITY;
	distance_array[0][1] = INFINITY;
	distance_array[1][0] = INFINITY;
	distance_array[1][1] = 0;

	// initialize the first two rows
	for(c = 2; c < len2 + 2; c++) {
		distance_array[0][c] = INFINITY;
		distance_array[1][c] = distance_array[1][c-1] + insertionWeight;
	}

	// build the matrix - always keep the 3 last rows calculated in the matrix
	for(int r = 2; r < len1+2; r++) {
		// initialize the 1st element in the row
		distance_array[r][0] = INFINITY;
		// initialize the 2nd element in the row
		distance_array[r][1] = distance_array[r-1][1] + deletionWeight;

		// stores the last match position in the current row
		// initialy points to the 2nd column so the the previous
		// is the infinity column
		int last_match_col = 1;

		// calculate the rest of the row
		for(int c = 2; c < len2 + 2; c++) {
			double substitutionCost;
			bool is_match = S2[c-2] == S1[r-2];

			// save last_match_col before it might be updated
			int _last_match_col = last_match_col;
			if (is_match) {
				substitutionCost = 0;
				last_match_col = c;
			} else {
				substitutionCost = substitutionWeight;
			}
			double min = fmin(fmin(fmin
				(
					distance_array[r - 1][c] + deletionWeight,  /* deletion from S1 */
					distance_array[r][c-1] + insertionWeight    /* insertion to S1 */
				),
					distance_array[r-1][c-1] + substitutionCost  /* substitution */
				),
					/* transposition */
					/* the upper left of the closest uper left position we can transpose with
					   (cost before transposition) */
					distance_array[last_match_row[c] - 1][_last_match_col - 1] +
					/* Proved in From Damerau, Fred J. (March 1964),
					   "A technique for computer detection and correction of spelling errors"
					   And the fact that we assert that: 2Wt >= Wi + Wd there can only be
					   deletions xor insertions between the tranposed characters.
					   Thus, we need to consider only two symmetric ways of modifying a substring more than once:
					   (1) transpose letters and insert an arbitrary number of characters between them, or
					   (2) delete a sequence of characters and transpose letters that become adjacent after deletion.
					*/
					(r - last_match_row[c] - 1)*deletionWeight +
					(c - _last_match_col - 1)*insertionWeight +
					transpositionWeight
				);
			distance_array[r][c] = min;

			if(is_match) {
				last_match_row[c] = r;
			}
		}
	}

	double res = distance_array[len1 + 1][len2 + 1];
	for(r = 0; r < len1 + 2; r++) {
		rm_free(distance_array[r]);
	}
	rm_free(distance_array);
	rm_free(last_match_row);
	rm_free(S1);
	rm_free(S2);
	return res;
}

static bool lev_distance_parse_params
(
	const struct Pair *distFuncParams,
	double *insertionWeight,
	double *deletionWeight,
	double *substitutionWeight,
	double *transpositionWeight
) {
	if(distFuncParams != NULL) {
		for(int i = 0; i < array_len((void *)distFuncParams); i++) {
			SIValue k = distFuncParams[i].key;
			SIValue v = distFuncParams[i].val;
			if(SI_TYPE(k) != T_STRING) {
				Error_SITypeMismatch(k, T_STRING);
				return false;
			}

			if(SI_TYPE(v) != T_DOUBLE) {
				Error_SITypeMismatch(v, T_DOUBLE);
				return false;
			}

			if(strcasecmp(k.stringval, "InsertionWeight") == 0) {
				*insertionWeight = v.doubleval;
			} else if(strcasecmp(k.stringval, "DeletionWeight") == 0) {
				*deletionWeight = v.doubleval;
			} else if(strcasecmp(k.stringval, "SubstitutionWeight") == 0) {
				*substitutionWeight = v.doubleval;
			} else if(transpositionWeight && 
				strcasecmp(k.stringval, "TranspositionWeight") == 0) {
				*transpositionWeight = v.doubleval;
			} else {
				ErrorCtx_RaiseRuntimeException
				("ArgumentError: map argument to string.distance() has an invalid key");
				return false;
			}
		}

		if(*insertionWeight < 0.0 ||
			*deletionWeight < 0.0 ||
			*substitutionWeight < 0.0 ||
			(transpositionWeight && *transpositionWeight < 0.0))
		{
			ErrorCtx_RaiseRuntimeException
			("ArgumentError: string.distance(), weight has a negative weight");
			return false;
		}

		if(isnan(*insertionWeight) ||
			isnan(*deletionWeight) ||
			isnan(*substitutionWeight) ||
			(transpositionWeight && isnan(*transpositionWeight)))
		{
			ErrorCtx_RaiseRuntimeException
			("ArgumentError: string.distance(), weight has a nan value");
			return false;
		}
	}

	return true;
}

static double hamming_distance
(
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2
) {
	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	if(len1 != len2) {
		rm_free(S1);
		rm_free(S2);
		return -1;
	}

	size_t distance = 0;
	for(int i = 0; i < len1; i++) {
		if(S1[i] != S2[i]) {
			distance++;
		}
	}

	rm_free(S1);
	rm_free(S2);
	return distance;
}

static double jaro_similarity_Int32
(
	int32_t *S1,
	int32_t *S2,
	size_t len1,
	size_t len2
) {

	// S1 should be the shorter string,
	// we allowed cause it's a symmetric function
	if(len1 > len2) {
		__SWAP(int32_t *, S1, S2);
		__SWAP(size_t, len1, len2);
	}

	if(len2 == 0) {
		// both stings are empty
		return 1.0;
	} else if(len2 == 1) {
		return S1[0] == S2[0] ? 1 : 0;
	}

    // maximum distance upto which matching
    // is allowed
	size_t range = MAX(0, (len2 / 2 - 1));
	bool *matchIndexes = (bool*)rm_calloc(len2, sizeof(bool));
	char *v1 = (char*)rm_malloc(len1*sizeof(char));
	size_t n_match = 0;
	size_t i, j;

	// for each character in S1,
	// find if there is any matching character in range
	for(i = 0; i < len1; i++) {
		size_t end = MIN(len2 - 1, i + range);
		for(j = (i > range) ? i - range : 0; j <= end; j++) {
			if(S1[i] == S2[j] && !matchIndexes[j]) {
				v1[n_match] = S1[i];
				matchIndexes[j] = true;
				n_match++;
				break;
			}
		}
	}

	char *v2 = (char*)rm_malloc(n_match);
	for(i = 0, j = 0; i < len2; i++) {
		if(matchIndexes[i]) {
			v2[j] = S2[i];
			j++;
		}
	}

	// find the number of transpositions
	size_t n_transpositions = 0;
	for(i = 0; i < n_match; i++) {
		if(v1[i] != v2[i]) {
			n_transpositions++;
		}
	}

	rm_free(v1);
	rm_free(v2);
	rm_free(matchIndexes);

	// if no matches found, similarity is 0
	if(n_match == 0) {
		return 0;
	}

	// return the Jaro similarity
	return ((double)n_match/len1 + (double)n_match/len2 + 
	(n_match - ((double)n_transpositions/2))/n_match)/3;
}

static double jaro_similarity
(
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2
) {
	if(_len1 == 0 && _len2 == 0) {
		// both stings are empty
		return 1.0;
	}

	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	double res = jaro_similarity_Int32(S1, S2, len1, len2);

	rm_free(S1);
	rm_free(S2);
	return res;
}

static bool jaro_winkler_parse_params(const struct Pair *distFuncParams, double *scaleFactor, double *threshold) {
	if(distFuncParams != NULL) {
		for(int i = 0; i < array_len((void *)distFuncParams); i++) {
			SIValue k = distFuncParams[i].key;
			SIValue v = distFuncParams[i].val;
			if(SI_TYPE(k) != T_STRING) {
				Error_SITypeMismatch(k, T_STRING);
				return false;
			}

			if(SI_TYPE(v) != T_DOUBLE) {
				Error_SITypeMismatch(v, T_DOUBLE);
				return false;
			}

			if(strcasecmp(k.stringval, "ScaleFactor") == 0) {
				*scaleFactor = v.doubleval;

				if(unlikely(*scaleFactor < 0.0 || *scaleFactor > 0.25)) {
					ErrorCtx_RaiseRuntimeException
					("ArgumentError: string.distance(), scaleFactor value is out of bounds");
					return false;
				}

				if(unlikely(isnan(*scaleFactor))) {
					ErrorCtx_RaiseRuntimeException
					("ArgumentError: string.distance(), scaleFactor value is nan");
					return false;
				}
			} else if(strcasecmp(k.stringval, "threshold") == 0) {
				*threshold = v.doubleval;

				if(unlikely(*threshold < 0.0 || *threshold > 1.0)) {
					ErrorCtx_RaiseRuntimeException
					("ArgumentError: string.distance(), threshold value is out of bounds");
					return false;
				}

				if(unlikely(isnan(*threshold))) {
					ErrorCtx_RaiseRuntimeException
					("ArgumentError: string.distance(), threshold value is nan");
					return false;
				}
			} else {
				ErrorCtx_RaiseRuntimeException
				("ArgumentError: map argument to string.distance() has an invalid key");
				return false;
			}
		}
	}

	return true;
}

static double jaro_winkler_distance(
	const char *_S1,
	const char *_S2,
	size_t _len1,
	size_t _len2,
	double scaleFactor,
	double threshold
) {

	size_t len1, len2;
	int32_t *S1, *S2;
	S1 = str_toInt32(_S1, _len1, &len1);
	S2 = str_toInt32(_S2, _len2, &len2);

	// S1 should be the shorter string,
	// it's allowed cause it's a symmetric function
	if(len1 > len2) {
		__SWAP(int32_t *, S1, S2);
		__SWAP(size_t, len1, len2);
	}
	double j = jaro_similarity_Int32(S1, S2, len1, len2);

	if(j == 0) {
		rm_free(S1);
		rm_free(S2);
		return 1.0;
	}

	// find the number of common prefix characters
	size_t prefix = 0;
	for(size_t i = 0; i < MIN(len1,4); i++) {
		if(S1[i] == S2[i]) {
			prefix++;
		} else {
			break;
		}
	}

	rm_free(S1);
	rm_free(S2);

	// calculate the Jaro-Winkler similarity
	double sim_w = (j < threshold) ? j : j + scaleFactor * prefix * (1.0 - j);

	// return the Jaro-Winkler distance
	return 1.0 - sim_w;
}

// given two strings, return their distance according to a specified distance function.
// string.distance(str1, str2, distFunc = 'Lev', distFuncParams = {}) → floating-point
SIValue AR_DISTANCE_STR(SIValue *argv, int argc, void *private_data) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) {
		return SI_NullVal();
	}

	const char *S1 = argv[0].stringval;
	const char *S2 = argv[1].stringval;
	size_t      len1       = strlen(S1);
	size_t      len2       = strlen(S2);

	char *distFunc = "Lev";
	if(argc > 2) {
		distFunc = argv[2].stringval;
	}

	const struct Pair *distFuncParams = NULL;
	if(argc > 3) {
		distFuncParams = argv[3].map;
	}

	double res;

	if (strcasecmp(distFunc, "Lev") == 0) {
		double insertionWeight = 1;
		double deletionWeight = 1;
		double substitutionWeight = 1;

		if(!lev_distance_parse_params(
			distFuncParams,
			&insertionWeight,
			&deletionWeight,
			&substitutionWeight,
			NULL)
		) {
			return SI_NullVal();
		}

		res = levenshtein_distance(
			S1,
			S2,
			len1,
			len2,
			insertionWeight,
			deletionWeight,
			substitutionWeight
		);
	} else if (strcasecmp(distFunc, "OSA") == 0) {
		double insertionWeight = 1;
		double deletionWeight = 1;
		double substitutionWeight = 1;
		double transpositionWeight = 1;

		if(!lev_distance_parse_params(
			distFuncParams,
			&insertionWeight,
			&deletionWeight,
			&substitutionWeight,
			&transpositionWeight)
		) {
			return SI_NullVal();
		}

		res = damerau_levenshtein_distance_osa(
			S1,
			S2,
			len1,
			len2,
			insertionWeight,
			deletionWeight,
			substitutionWeight,
			transpositionWeight
		);
	} else if (strcasecmp(distFunc, "DamLev") == 0) {
		double insertionWeight = 1;
		double deletionWeight = 1;
		double substitutionWeight = 1;
		double transpositionWeight = 1;

		if(!lev_distance_parse_params(
			distFuncParams,
			&insertionWeight,
			&deletionWeight,
			&substitutionWeight,
			&transpositionWeight)
		) {
			return SI_NullVal();
		}

		if(2*transpositionWeight < insertionWeight + deletionWeight) {
			ErrorCtx_RaiseRuntimeException
			("ArgumentError: map argument to string.distance() has an "
			"invalid key: 2*TranspositionWeight needs to be greater or "
			"equal InsertionWeight + DeletionWeight");
			return SI_NullVal();
		}

		res = damerau_levenshtein_distance(
			S1,
			S2,
			len1,
			len2,
			insertionWeight,
			deletionWeight,
			substitutionWeight,
			transpositionWeight
		);
	} else if (strcasecmp(distFunc, "Ham") == 0) {
		res = hamming_distance(S1, S2, len1, len2);
	} else if (strcasecmp(distFunc, "Jaro") == 0) {
		// Jaro distance is 1 - Jaro similarity
		res = 1.0 - jaro_similarity(S1, S2, len1, len2);
	} else if (strcasecmp(distFunc, "JaroW") == 0) {
		double scaleFactor = 0.1;
		double threshold = 0.7;
		if(!jaro_winkler_parse_params(
			distFuncParams,
			&scaleFactor,
			&threshold)
		) {
			return SI_NullVal();
		}
		res = jaro_winkler_distance(S1, S2, len1, len2, scaleFactor, threshold);
	} else {
		ErrorCtx_RaiseRuntimeException
		("ArgumentError: string.distance() has an invalid distance function");
		return SI_NullVal();
	}


	return 	SI_DoubleVal(res);
}

//==============================================================================
//=== Scalar functions =========================================================
//==============================================================================

SIValue AR_RANDOMUUID(SIValue *argv, int argc, void *private_data) {
	char *uuid = UUID_New();
	return SI_TransferStringVal(uuid);
}

void Register_StringFuncs() {
	SIType *types;
	SIType ret_type;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, T_INT64 | T_NULL);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("left", AR_LEFT, 2, 2, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("ltrim", AR_LTRIM, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, T_INT64 | T_NULL);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("right", AR_RIGHT, 2, 2, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("rtrim", AR_RTRIM, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_ARRAY | T_NULL));
	ret_type = T_STRING | T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("reverse", AR_REVERSE, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, T_INT64);
	array_append(types, T_INT64);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("substring", AR_SUBSTRING, 2, 3, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_ARRAY | T_NULL));
	array_append(types, T_STRING);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("string.join", AR_JOIN, 1, 2, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("string.matchRegEx", AR_MATCHREGEX, 2, 2, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("string.replaceRegEx", AR_REPLACEREGEX, 2, 3, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("tolower", AR_TOLOWER, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("toupper", AR_TOUPPER, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, STRINGABLE | T_NULL);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("tostring", AR_TOSTRING, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("tostringornull", AR_TOSTRING, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("tojson", AR_TOJSON, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("trim", AR_TRIM, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_BOOL | T_NULL;
	func_desc = AR_FuncDescNew("contains", AR_CONTAINS, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_BOOL | T_NULL;
	func_desc = AR_FuncDescNew("starts with", AR_STARTSWITH, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_BOOL | T_NULL;
	func_desc = AR_FuncDescNew("ends with", AR_ENDSWITH, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 0);
	ret_type = T_STRING;
	func_desc = AR_FuncDescNew("randomuuid", AR_RANDOMUUID, 0, 0, types, ret_type, false, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_STRING | T_NULL;
	func_desc = AR_FuncDescNew("replace", AR_REPLACE, 3, 3, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("split", AR_SPLIT, 2, 2, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 4);
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING | T_NULL));
	array_append(types, (T_STRING));
	array_append(types, (T_MAP));
	ret_type = T_DOUBLE | T_NULL;
	func_desc = AR_FuncDescNew("string.distance", AR_DISTANCE_STR, 2, 4, types, ret_type, false, true);
	AR_RegFunc(func_desc);
}

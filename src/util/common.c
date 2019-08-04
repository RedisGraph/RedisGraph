#include "common.h"
#include "assert.h"
#include <ctype.h>
#include <string.h>

void _toLower(const char *str, char *lower, size_t *lower_len) {
	size_t str_len = strlen(str);
	/* Avoid overflow. */
	assert(*lower_len > str_len);

	/* Update lower len*/
	*lower_len = str_len;

	int i = 0;
	for(; i < str_len; i++) lower[i] = tolower(str[i]);
	lower[i] = 0;
}
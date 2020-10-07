//
// Created by alon on 07/10/2020.
//
#include "strutil.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>


void str_tolower(const char *str, char *lower, short *lower_len) {
    size_t str_len = strlen(str);
    /* Avoid overflow. */
    assert(*lower_len > str_len);

    /* Update lower len*/
    *lower_len = str_len;

    size_t i = 0;
    for(; i < str_len; i++) lower[i] = tolower(str[i]);
    lower[i] = 0;
}

void str_toupper(const char *str, char *upper, short *upper_len) {
    size_t str_len = strlen(str);
    /* Avoid overflow. */
    assert(*upper_len > str_len);

    /* Update lower len*/
    *upper_len = str_len;
    size_t i = 0;
    for(; i < str_len; i++) upper[i] = toupper(str[i]);
    upper[i] = 0;
}
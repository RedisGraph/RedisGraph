/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <glob.h>

//convert str to a lower case string, save it in lower and return lower
void str_tolower(const char *str, char *lower, size_t *lower_len);

//convert str to a n upper case string, save it in upper and return upper
void str_toupper(const char *str, char *upper, size_t *upper_len);

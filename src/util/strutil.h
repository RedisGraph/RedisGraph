/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

//convert str to a lower case string and save it in lower
void str_tolower(const char *str, char *lower, size_t *lower_len);

//convert str to a n upper case string and save it in upper
void str_toupper(const char *str, char *upper, size_t *upper_len);


// Utility function to increase the size of a buffer.
void str_ExtendBuffer(
	char **buf,           // buffer to populate
	size_t *bufferLen,    // size of buffer
	size_t extensionLen   // number of bytes to add
);


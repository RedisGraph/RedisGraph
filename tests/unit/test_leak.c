/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <stdlib.h>
#include <stdint.h>

int setup() {
}

#define TEST_INIT setup();
#include "acutest.h"

///////////////////////////////////////////////////////////////////////////////////////////

void test_leak1() {
	void *p = malloc(100);
}

//-----------------------------------------------------------------------------------------

typedef struct {
    int32_t *data;
    int32_t length;
} List;

List *resizeArray(List *array) {
    int32_t *p = array->data;
    p = realloc(p, 15 * sizeof(int32_t)); // doesn't update array->data
    return array;
}

void test_leak2() {
    List *p = calloc(1, sizeof(List));
    p->data = calloc(10, sizeof(int32_t));
    p = resizeArray(p);
    free(p->data);
    free(p);
}

//-----------------------------------------------------------------------------------------

typedef struct {
  int n;
  int *nums;
} Numbers;

int n;
Numbers *numbers;

void create_numbers(Numbers **nrs, int *n) {
  *n = 3;
  *nrs = malloc((sizeof(Numbers) * 3));
  Numbers *nm = *nrs;
  for (int i = 0; i < 3; ++i) {
      nm->n = i + 1;
      nm->nums = malloc(sizeof(int) * (i + 1));
      for (int j = 0; j < i + 1; ++j)
        nm->nums[j] = i + j;
      nm++;
  }
}

void test_possibly_lost() { 
  create_numbers(&numbers, &n);
  for (int i = 0; i < 2; i++) {
      numbers++;
  }
}

//-----------------------------------------------------------------------------------------

char *create_banner() {
   const char *user = getenv("USER");
   size_t len = 1 + 2 * 4 + strlen (user) + 1;
   char *b = malloc(len);
   sprintf (b, "\t|** %s **|", user);
   return b;
 }
 
char *banner;

void test_reachable() {
  banner = create_banner();
}

//-----------------------------------------------------------------------------------------

void test_bad_read() {
    char* destination = calloc(27, sizeof(char));
    char* source = malloc(26 * sizeof(char));

    for (uint8_t i = 0; i < 27; ++i) {
        *(destination + i) = *(source + i); //Look at the last iteration.
    }

    free(destination);
    free(source);
}

//-----------------------------------------------------------------------------------------

void test_bad_write() {
    char* alphabet = calloc(26, sizeof(char));

    for(uint8_t i = 0; i < 26; ++i) {
        *(alphabet + i) = 'A' + i;
    }
    *(alphabet + 26) = '\0'; // null-terminate the string?

    free(alphabet);
}

//-----------------------------------------------------------------------------------------

char *func1(void) { return malloc(256); }
char *func2(void) { return func1(); }

void test_track() {
    char *ptr = func2();
    if (*ptr) {
        printf("foo\n");
    }
}

//-----------------------------------------------------------------------------------------

TEST_LIST = {
	{ "leak1", test_leak1},
	{ "leak2", test_leak2},
	{ "possibly_lost", test_possibly_lost},
    { "reachable", test_reachable},
    { "bad_read", test_bad_read},
    { "bad_write", test_bad_write},
    { "track", test_track},
	{ NULL, NULL }
};

///////////////////////////////////////////////////////////////////////////////////////////

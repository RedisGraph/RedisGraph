/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/value.h"

void test_value_parse() {
    SIValue v;
    char *str = "12345";
    v = SIValue_FromString(str);
    assert(v.type == T_DOUBLE);
    assert(v.doubleval == 12345);

    str = "3.14";
    v = SIValue_FromString(str);
    assert(v.type == T_DOUBLE);
    
    /* Almost equals. */
    assert((v.doubleval - 3.14) < 0.0001);

    str = "-9876";
    v = SIValue_FromString(str);
    assert(v.type == T_DOUBLE);
    assert(v.doubleval == -9876);

    str = "Test!";
    v = SIValue_FromString(str);
    assert(v.type == T_STRING);
    assert(strcmp(v.stringval, "Test!") == 0);
    SIValue_Free(&v);

    str = "+1.0E1";
    v = SIValue_FromString(str);
    assert(v.type == T_DOUBLE);
    assert(v.doubleval == 10);

    /* Out of double range */
    str = "1.0001e10001";
    v = SIValue_FromString(str);
    assert(v.type == T_STRING);
    assert(strcmp(v.stringval, "1.0001e10001") == 0);
    SIValue_Free(&v);
}

int main(int argc, char **argv) {
    test_value_parse();
    printf("test_value - PASS!\n");
    return 0;
}

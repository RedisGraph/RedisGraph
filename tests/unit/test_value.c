#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/value.h"

void test_value_parse() {
    SIValue v;
    char *str = "12345";
    SIValue_FromString(&v, str);
    assert(v.type == T_DOUBLE);
    assert(v.doubleval == 12345);

    str = "3.14";
    SIValue_FromString(&v, str);
    assert(v.type == T_DOUBLE);
    
    /* Almost equals. */
    assert((v.doubleval - 3.14) < 0.0001);

    str = "-9876";
    SIValue_FromString(&v, str);
    assert(v.type == T_DOUBLE);
    assert(v.doubleval == -9876);

    str = "Test!";
    SIValue_FromString(&v, str);
    assert(v.type == T_STRING);
    assert(strcmp(v.stringval, "Test!") == 0);
}

int main(int argc, char **argv) {
    test_value_parse();
    printf("test_value - PASS!\n");
    return 0;
}
#include <stdio.h>
#include <string.h>
#include "../../src/arithmetic_expression.h"
#include "../../src/value.h"
#include "../../src/graph/node.h"
#include "../../src/aggregate/agg_funcs.h"
#include "assert.h"

void test_arithmetic_expression() {

    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    
    /* 1 */
    SIValue result = AR_EXP_Evaluate(one);
    assert(result.doubleval == 1);

    char* str = NULL;
    AR_EXP_ToString(one, &str);    
    assert(strcmp(str, "1.000000") == 0);
    free(str);
    str = NULL;

    /* 1+2*3 */
    AR_ExpNode *two = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
    AR_ExpNode *three = AR_EXP_NewConstOperandNode(SI_DoubleVal(3));
    AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
    AR_ExpNode *mul =  AR_EXP_NewOpNode("MUL", 2);

    add->op.children[0] = one;
    add->op.children[1] = mul;
    
    mul->op.children[0] = two;
    mul->op.children[1] = three;

    result = AR_EXP_Evaluate(add);
    assert(result.doubleval == 7);
    AR_EXP_Free(add);

    /* 1 + 1 + 1 + 1 + 1 + 1 */
    one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    AR_ExpNode *add_1 = AR_EXP_NewOpNode("ADD", 2);
    AR_ExpNode *add_2 = AR_EXP_NewOpNode("ADD", 2);
    AR_ExpNode *add_3 = AR_EXP_NewOpNode("ADD", 2);
    AR_ExpNode *add_4 = AR_EXP_NewOpNode("ADD", 2);
    AR_ExpNode *add_5 = AR_EXP_NewOpNode("ADD", 2);

    add_4->op.children[0] = one;
    add_4->op.children[1] = one;
    add_5->op.children[0] = one;
    add_5->op.children[1] = one;

    add_3->op.children[0] = one;
    add_3->op.children[1] = add_5;

    add_2->op.children[0] = add_4;
    add_2->op.children[1] = one;

    add_1->op.children[0] = add_2;
    add_1->op.children[1] = add_3;

    result = AR_EXP_Evaluate(add_1);
    assert(result.doubleval == 6);

    /* Don't free as one is referenced multiple times. */
    // AR_EXP_Free(add_1);

    /* ABS(-5 + 2 * 1) */
    AR_ExpNode *minus_five = AR_EXP_NewConstOperandNode(SI_DoubleVal(-5));
    two = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
    add = AR_EXP_NewOpNode("ADD", 2);
    mul =  AR_EXP_NewOpNode("MUL", 2);
    AR_ExpNode *absolute = AR_EXP_NewOpNode("ABS", 1);

    absolute->op.children[0] = add;
    
    add->op.children[0] = minus_five;
    add->op.children[1] = mul;
    
    mul->op.children[0] = two;
    mul->op.children[1] = one;

    result = AR_EXP_Evaluate(absolute);
    assert(result.doubleval == 3);
}

void test_variadic_arithmetic_expression() {
    /* person.age += 1 */
    Node *node = NewNode(1, "person");
    char *props[2] = {"age", "name"};
    SIValue vals[2] = {SI_DoubleVal(33), SI_StringValC("joe")};
    GraphEntity_Add_Properties((GraphEntity*)node, 2, props, vals);

    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    AR_ExpNode *person = AR_EXP_NewVariableOperandNode((GraphEntity **)(&node), "age", "joe");
    AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
    add->op.children[0] = one;
    add->op.children[1] = person;

    SIValue result = AR_EXP_Evaluate(person);
    assert(result.doubleval == 33);

    result = AR_EXP_Evaluate(one);
    assert(result.doubleval == 1);

    result = AR_EXP_Evaluate(add);
    assert(result.doubleval == 34);
    AR_EXP_Free(add);
}

void test_aggregated_arithmetic_expression() {
    /* SUM(1) */
    AR_ExpNode *sum = AR_EXP_NewOpNode("SUM", 1);
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    sum->op.children[0] = one;
    
    AR_EXP_Aggregate(sum);
    AR_EXP_Aggregate(sum);
    AR_EXP_Aggregate(sum);

    AR_EXP_Reduce(sum);
    SIValue result = AR_EXP_Evaluate(sum);
    assert(result.doubleval == 3);

    /* 2+SUM(1) */
    AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
    sum = AR_EXP_NewOpNode("SUM", 1);
    AR_ExpNode *two = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
    sum->op.children[0] = one;
    add->op.children[0] = two;
    add->op.children[1] = sum;
    AR_EXP_Aggregate(add);
    AR_EXP_Aggregate(add);
    AR_EXP_Aggregate(add);

    AR_EXP_Reduce(add);

    /* Just for the kick of it, call reduce more than once.*/
    AR_EXP_Reduce(add);
    result = AR_EXP_Evaluate(add);
    assert(result.doubleval == 5);
    AR_EXP_Free(add);
}

void _test_string(const AR_ExpNode *exp, const char *expected) {
    char *str;
    AR_EXP_ToString(exp, &str);
    // printf("str: %s\n", str);
    assert(strcmp(str, expected) == 0);
    free(str);
}

void test_string_representation() {
    /* Const. */
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    _test_string(one, "1.000000");

    /* Variadic. */
    AR_ExpNode *person = AR_EXP_NewVariableOperandNode(NULL, "age", "joe");
    _test_string(person, "joe.age");
    
    /* Aggregation. */
    AR_ExpNode *sum = AR_EXP_NewOpNode("SUM", 1);
    sum->op.children[0] = one;
    _test_string(sum, "SUM(1.000000)");
    
    /* Function. */
    AR_ExpNode *absolute = AR_EXP_NewOpNode("ABS", 1);
    absolute->op.children[0] = one;
    _test_string(absolute, "ABS(1.000000)");
    
    /* Nested. */
    absolute = AR_EXP_NewOpNode("ABS", 1);
    AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
    
    add->op.children[0] = one;
    add->op.children[1] = sum;
    sum->op.children[0] = person;
    absolute->op.children[0] = add;
    _test_string(absolute, "ABS(1.000000 + SUM(joe.age))");
}

void _test_ar_func(AR_ExpNode *root, SIValue expected) {
    SIValue res = AR_EXP_Evaluate(root);
    
    if(res.doubleval != expected.doubleval) {
        printf("res.doubleval: %lf expected.doubleval: %lf\n", res.doubleval, expected.doubleval);
    }

    assert(res.doubleval == expected.doubleval);
}

void test_abs() {
    AR_ExpNode *root = AR_EXP_NewOpNode("ABS", 1);
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    AR_ExpNode *minus_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(-1));
    AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
    
    root->op.children[0] = one;
    SIValue expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);

    root->op.children[0] = minus_one;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);
    
    root->op.children[0] = zero;
    expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);
}

void test_ceil() {
    AR_ExpNode *root = AR_EXP_NewOpNode("CEIL", 1);
    AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));

    root->op.children[0] = half;
    SIValue expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);

    root->op.children[0] = one;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);

    root->op.children[0] = dot_one;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);
}

void test_floor() {
    AR_ExpNode *root = AR_EXP_NewOpNode("FLOOR", 1);
    AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
    AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));

    /* floor(0.5) */
    root->op.children[0] = half;
    SIValue expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);

    /* floor(1) */
    root->op.children[0] = one;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);

    /* floor(0.1) */
    root->op.children[0] = dot_one;
    expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);
}

void test_rand() {
    AR_ExpNode *root = AR_EXP_NewOpNode("RAND", 0);
    for(int i = 0; i < 10; i++) {
        SIValue res = AR_EXP_Evaluate(root);
        assert(res.doubleval >= 0 && res.doubleval <=1);
    }
}

void test_round() {
    AR_ExpNode *root = AR_EXP_NewOpNode("ROUND", 1);
    AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
    AR_ExpNode *dot_four = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.49));
    AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

    /* round(0) */
    root->op.children[0] = zero;
    SIValue expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);

    /* round(0.49) */
    root->op.children[0] = dot_four;
    expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);

    /* round(0.5) */
    root->op.children[0] = half;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);

    /* round(1) */
    root->op.children[0] = one;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);
}

void test_sign() {
    
    AR_ExpNode *root = AR_EXP_NewOpNode("SIGN", 1);
    AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
    AR_ExpNode *negative = AR_EXP_NewConstOperandNode(SI_DoubleVal(-8));
    AR_ExpNode *positive = AR_EXP_NewConstOperandNode(SI_DoubleVal(2.3));

    /* sign(0) */
    root->op.children[0] = zero;
    SIValue expected = SI_DoubleVal(0);
    _test_ar_func(root, expected);

    /* sign(-) */
    root->op.children[0] = negative;
    expected = SI_DoubleVal(-1);
    _test_ar_func(root, expected);

    /* sign(+) */
    root->op.children[0] = positive;
    expected = SI_DoubleVal(1);
    _test_ar_func(root, expected);
}

void test_left() {
    AR_ExpNode *root = AR_EXP_NewOpNode("LEFT", 2);
    AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));
    AR_ExpNode *left = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
    AR_ExpNode *entire_string_len = AR_EXP_NewConstOperandNode(SI_DoubleVal(100));

    root->op.children[0] = str;
    root->op.children[1] = left;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "much";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = str;
    root->op.children[1] = entire_string_len;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_reverse() {
    AR_ExpNode *root = AR_EXP_NewOpNode("REVERSE", 1);
    AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));
    AR_ExpNode *empty_str = AR_EXP_NewConstOperandNode(SI_StringValC(""));
    
    root->op.children[0] = str;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "ohcahcum";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = empty_str;
    result = AR_EXP_Evaluate(root);
    expected = "";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_right() {
    AR_ExpNode *root = AR_EXP_NewOpNode("RIGHT", 2);
    AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));
    AR_ExpNode *right = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
    AR_ExpNode *entire_string_len = AR_EXP_NewConstOperandNode(SI_DoubleVal(100));

    root->op.children[0] = str;
    root->op.children[1] = right;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "acho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = str;
    root->op.children[1] = entire_string_len;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_ltrim() {
    AR_ExpNode *root = AR_EXP_NewOpNode("lTrim", 1);
    AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   muchacho"));
    AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho   "));
    AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   much   acho   "));
    AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));

    root->op.children[0] = left_spaced_str;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = right_spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho   ";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "much   acho   ";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = no_space_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_rtrim() {
    AR_ExpNode *root = AR_EXP_NewOpNode("rTrim", 1);
    AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   muchacho"));
    AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho   "));
    AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   much   acho   "));
    AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));

    root->op.children[0] = left_spaced_str;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "   muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = right_spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "   much   acho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = no_space_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_substring() {
    AR_ExpNode *root = AR_EXP_NewOpNode("SUBSTRING", 3);
    AR_ExpNode *original_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));
    AR_ExpNode *start = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
    AR_ExpNode *length = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
    AR_ExpNode *start_middel = AR_EXP_NewConstOperandNode(SI_DoubleVal(3));
    AR_ExpNode *length_overflow = AR_EXP_NewConstOperandNode(SI_DoubleVal(20));

    root->op.children[0] = original_str;
    root->op.children[1] = start;
    root->op.children[2] = length;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "much";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = original_str;
    root->op.children[1] = start_middel;
    root->op.children[2] = length_overflow;
    result = AR_EXP_Evaluate(root);
    expected = "hacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_tolower() {
    AR_ExpNode *root = AR_EXP_NewOpNode("toLower", 1);
    AR_ExpNode *str1 = AR_EXP_NewConstOperandNode(SI_StringValC("MuChAcHo"));
    AR_ExpNode *str2 = AR_EXP_NewConstOperandNode(SI_StringValC("mUcHaChO"));

    root->op.children[0] = str1;    
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = str2;    
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);   
}

void test_toupper() {
    AR_ExpNode *root = AR_EXP_NewOpNode("toUpper", 1);
    AR_ExpNode *str1 = AR_EXP_NewConstOperandNode(SI_StringValC("MuChAcHo"));
    AR_ExpNode *str2 = AR_EXP_NewConstOperandNode(SI_StringValC("mUcHaChO"));

    root->op.children[0] = str1;    
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "MUCHACHO";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = str2;    
    result = AR_EXP_Evaluate(root);
    expected = "MUCHACHO";
    assert(strcmp(result.stringval.str, expected) == 0);   
}

void test_toString() {
    AR_ExpNode *root = AR_EXP_NewOpNode("toString", 1);
    AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));
    AR_ExpNode *number = AR_EXP_NewConstOperandNode(SI_DoubleVal(3.14));

    root->op.children[0] = str;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = number;
    result = AR_EXP_Evaluate(root);
    expected = "3.140000";
    assert(strcmp(result.stringval.str, expected) == 0);
}

void test_trim() {
    AR_ExpNode *root = AR_EXP_NewOpNode("trim", 1);
    AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   muchacho"));
    AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho   "));
    AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringValC("   much   acho   "));
    AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringValC("muchacho"));

    root->op.children[0] = left_spaced_str;
    SIValue result = AR_EXP_Evaluate(root);
    char *expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = right_spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = spaced_str;
    result = AR_EXP_Evaluate(root);
    expected = "much   acho";
    assert(strcmp(result.stringval.str, expected) == 0);

    root->op.children[0] = no_space_str;
    result = AR_EXP_Evaluate(root);
    expected = "muchacho";
    assert(strcmp(result.stringval.str, expected) == 0);
}

int main(int argc, char **argv) {
    AR_RegisterFuncs();
    Agg_RegisterFuncs();

    test_arithmetic_expression();
    test_variadic_arithmetic_expression();
    test_aggregated_arithmetic_expression();
    test_string_representation();
    
    /* test individual math functions.*/
    test_abs();
    test_ceil();
    test_floor();
    test_rand();
    test_round();
    test_sign();

    test_left();
    test_reverse();
    test_right();
    test_ltrim();
    test_rtrim();
    test_substring();
    test_tolower();
    test_toupper();
    test_toString();
    test_trim();

    printf("test_arithmetic_expression - PASS!\n");
    return 0;
}
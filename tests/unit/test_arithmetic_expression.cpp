/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/value.h"
#include "../../src/graph/node.h"
#include "../../src/arithmetic/agg_funcs.h"
#include "../../src/execution_plan/record.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

class ArithmeticTest: public ::testing::Test {
  protected:
    Record emptyRecord = Record_Empty();
    static void SetUpTestCase() {
      // Use the malloc family for allocations
      Alloc_Reset();

      AR_RegisterFuncs();
      Agg_RegisterFuncs();
    }

    static void TearDownTestCase() {
    }
};

void _test_string(const AR_ExpNode *exp, const char *expected) {
  char *str;
  AR_EXP_ToString(exp, &str);
  EXPECT_STREQ(str, expected);
  free(str);
}

void _test_ar_func(AR_ExpNode *root, SIValue expected, const Record r) {
  SIValue res = AR_EXP_Evaluate(root, r);
  EXPECT_EQ(res.doubleval, expected.doubleval);
}

TEST_F(ArithmeticTest, ExpressionTest) {  
  /* muchacho */
  AR_ExpNode *string = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  SIValue result = AR_EXP_Evaluate(string, emptyRecord);
  EXPECT_STREQ(result.stringval, "muchacho");
  AR_EXP_Free(string);

  /* 1 */
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  result = AR_EXP_Evaluate(one, emptyRecord);
  EXPECT_EQ(result.doubleval, 1);

  /* 1+2*3 */
  AR_ExpNode *two = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
  AR_ExpNode *three = AR_EXP_NewConstOperandNode(SI_DoubleVal(3));
  AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
  AR_ExpNode *mul =  AR_EXP_NewOpNode("MUL", 2);

  add->op.children[0] = one;
  add->op.children[1] = mul;

  mul->op.children[0] = two;
  mul->op.children[1] = three;

  result = AR_EXP_Evaluate(add, emptyRecord);
  EXPECT_EQ(result.doubleval, 7);
  AR_EXP_Free(add);

  /* 1 + 1 + 1 + 1 + 1 + 1 */
  AR_ExpNode *add_1 = AR_EXP_NewOpNode("ADD", 2);
  AR_ExpNode *add_2 = AR_EXP_NewOpNode("ADD", 2);
  AR_ExpNode *add_3 = AR_EXP_NewOpNode("ADD", 2);
  AR_ExpNode *add_4 = AR_EXP_NewOpNode("ADD", 2);
  AR_ExpNode *add_5 = AR_EXP_NewOpNode("ADD", 2);

  add_4->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  add_4->op.children[1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  add_5->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  add_5->op.children[1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

  add_3->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  add_3->op.children[1] = add_5;

  add_2->op.children[0] = add_4;
  add_2->op.children[1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

  add_1->op.children[0] = add_2;
  add_1->op.children[1] = add_3;

  result = AR_EXP_Evaluate(add_1, emptyRecord);
  EXPECT_EQ(result.doubleval, 6);

  AR_EXP_Free(add_1);

  /* ABS(-5 + 2 * 1) */
  AR_ExpNode *minus_five = AR_EXP_NewConstOperandNode(SI_DoubleVal(-5));
  add = AR_EXP_NewOpNode("ADD", 2);
  mul = AR_EXP_NewOpNode("MUL", 2);
  AR_ExpNode *absolute = AR_EXP_NewOpNode("ABS", 1);

  absolute->op.children[0] = add;

  add->op.children[0] = minus_five;
  add->op.children[1] = mul;

  mul->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
  mul->op.children[1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

  result = AR_EXP_Evaluate(absolute, emptyRecord);
  EXPECT_EQ(result.doubleval, 3);

  AR_EXP_Free(absolute);
}

TEST_F(ArithmeticTest, VariadicTest) {  
  /* person.age += 1 */
  Node *personNode = Node_New(1, "person", "joe");
  char *props[2] = {"age", "name"};
  SIValue vals[2] = {SI_DoubleVal(33), SI_StringVal("joe")};
  GraphEntity_Add_Properties((GraphEntity*)personNode, 2, props, vals);
  
  Record r = Record_Empty();
  Record_AddEntry(&r, personNode->alias, SI_PtrVal(personNode));

  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  AR_ExpNode *personAge = AR_EXP_NewVariableOperandNode("age", "joe");
  AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
  add->op.children[0] = one;
  add->op.children[1] = personAge;

  SIValue result = AR_EXP_Evaluate(personAge, r);
  EXPECT_EQ(result.doubleval, 33);

  result = AR_EXP_Evaluate(one, r);
  EXPECT_EQ(result.doubleval, 1);

  result = AR_EXP_Evaluate(add, r);
  EXPECT_EQ(result.doubleval, 34);

  Node_Free(personNode);
  AR_EXP_Free(add);
}

TEST_F(ArithmeticTest, AggregateTest) {
  /* SUM(1) */  
  AR_ExpNode *sum = AR_EXP_NewOpNode("SUM", 1);
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  sum->op.children[0] = one;

  AR_EXP_Aggregate(sum, emptyRecord);
  AR_EXP_Aggregate(sum, emptyRecord);
  AR_EXP_Aggregate(sum, emptyRecord);

  AR_EXP_Reduce(sum);
  SIValue result = AR_EXP_Evaluate(sum, emptyRecord);
  EXPECT_EQ(result.doubleval, 3);

  AR_EXP_Free(sum);
  one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

  /* 2+SUM(1) */
  AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);
  sum = AR_EXP_NewOpNode("SUM", 1);
  AR_ExpNode *two = AR_EXP_NewConstOperandNode(SI_DoubleVal(2));
  sum->op.children[0] = one;
  add->op.children[0] = two;
  add->op.children[1] = sum;
  AR_EXP_Aggregate(add, emptyRecord);
  AR_EXP_Aggregate(add, emptyRecord);
  AR_EXP_Aggregate(add, emptyRecord);

  AR_EXP_Reduce(add);

  /* Just for the kick of it, call reduce more than once.*/
  AR_EXP_Reduce(add);
  result = AR_EXP_Evaluate(add, emptyRecord);
  EXPECT_EQ(result.doubleval, 5);
  AR_EXP_Free(add);
}

TEST_F(ArithmeticTest, StringTest) {
  /* Const. */
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  _test_string(one, "1.000000");

  /* Variadic. */
  AR_ExpNode *person = AR_EXP_NewVariableOperandNode("age", "joe");
  _test_string(person, "joe.age");

  /* Aggregation. */
  AR_ExpNode *sum = AR_EXP_NewOpNode("SUM", 1);
  sum->op.children[0] = one;
  _test_string(sum, "SUM(1.000000)");

  /* Function. */
  AR_ExpNode *absolute = AR_EXP_NewOpNode("ABS", 1);
  absolute->op.children[0] = one;
  _test_string(absolute, "ABS(1.000000)");
  AR_EXP_Free(absolute);

  one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  /* Nested. */
  absolute = AR_EXP_NewOpNode("ABS", 1);
  AR_ExpNode *add = AR_EXP_NewOpNode("ADD", 2);

  add->op.children[0] = one;
  add->op.children[1] = sum;
  sum->op.children[0] = person;
  absolute->op.children[0] = add;
  _test_string(absolute, "ABS(1.000000 + SUM(joe.age))");

  AR_EXP_Free(absolute);
}

TEST_F(ArithmeticTest, AbsTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("ABS", 1);
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  AR_ExpNode *minus_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(-1));
  AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());  

  root->op.children[0] = one;
  SIValue expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = minus_one;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = zero;
  expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = null;
  expected = SI_NullVal();
  _test_ar_func(root, expected, emptyRecord);

  AR_EXP_Free(one);
  AR_EXP_Free(minus_one);
  AR_EXP_Free(zero);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, CeilTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("CEIL", 1);
  AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = half;
  SIValue expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = one;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = dot_one;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  root->op.children[0] = null;
  expected = SI_NullVal();
  _test_ar_func(root, expected, emptyRecord);
}

TEST_F(ArithmeticTest, FloorTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("FLOOR", 1);
  AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  /* floor(0.5) */
  root->op.children[0] = half;
  SIValue expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);

  /* floor(1) */
  root->op.children[0] = one;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);

  /* floor(0.1) */
  root->op.children[0] = dot_one;
  expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);

  /* floor(null) */
  root->op.children[0] = null;
  expected = SI_NullVal();
  _test_ar_func(root, expected, emptyRecord);

  AR_EXP_Free(root);
  AR_EXP_Free(half);
  AR_EXP_Free(one);
  AR_EXP_Free(dot_one);
}

TEST_F(ArithmeticTest, RoundTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("ROUND", 1);
  AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
  AR_ExpNode *dot_four = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.49));
  AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
  AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  /* round(0) */
  root->op.children[0] = zero;
  SIValue expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(zero);

  /* round(0.49) */
  root->op.children[0] = dot_four;
  expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(dot_four);

  /* round(0.5) */
  root->op.children[0] = half;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(half);

  /* round(1) */
  root->op.children[0] = one;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(one);

  /* null */
  root->op.children[0] = null;
  expected = SI_NullVal();
  _test_ar_func(root, expected, emptyRecord);

  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, SignTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("SIGN", 1);
  AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
  AR_ExpNode *negative = AR_EXP_NewConstOperandNode(SI_DoubleVal(-8));
  AR_ExpNode *positive = AR_EXP_NewConstOperandNode(SI_DoubleVal(2.3));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  /* sign(0) */
  root->op.children[0] = zero;
  SIValue expected = SI_DoubleVal(0);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(zero);

  /* sign(-) */
  root->op.children[0] = negative;
  expected = SI_DoubleVal(-1);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(negative);

  /* sign(+) */
  root->op.children[0] = positive;
  expected = SI_DoubleVal(1);
  _test_ar_func(root, expected, emptyRecord);
  AR_EXP_Free(positive);

  /* sign(null) */
  root->op.children[0] = null;
  expected = SI_NullVal();
  _test_ar_func(root, expected, emptyRecord);

  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, ReverseTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("REVERSE", 1);
  AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *empty_str = AR_EXP_NewConstOperandNode(SI_StringVal(""));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "ohcahcum";
  EXPECT_STREQ(result.stringval, expected);
  AR_EXP_Free(str);

  root->op.children[0] = empty_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "";
  EXPECT_STREQ(result.stringval, expected);
  AR_EXP_Free(empty_str);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, LeftTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("LEFT", 2);
  AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *left = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
  AR_ExpNode *entire_string_len = AR_EXP_NewConstOperandNode(SI_DoubleVal(100));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str;
  root->op.children[1] = left;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "much";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = str;
  root->op.children[1] = entire_string_len;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  root->op.children[1] = entire_string_len;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(left);
  AR_EXP_Free(str);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, RightTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("RIGHT", 2);
  AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *right = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
  AR_ExpNode *entire_string_len = AR_EXP_NewConstOperandNode(SI_DoubleVal(100));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str;
  root->op.children[1] = right;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "acho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = str;
  root->op.children[1] = entire_string_len;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  root->op.children[1] = entire_string_len;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(right);
  AR_EXP_Free(str);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, LTrimTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("lTrim", 1);
  AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   muchacho"));
  AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho   "));
  AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   much   acho   "));
  AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = left_spaced_str;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = right_spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho   ";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "much   acho   ";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = no_space_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(left_spaced_str);
  AR_EXP_Free(right_spaced_str);
  AR_EXP_Free(spaced_str);
  AR_EXP_Free(no_space_str);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, RTrimTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("rTrim", 1);
  AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   muchacho"));
  AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho   "));
  AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   much   acho   "));
  AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = left_spaced_str;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "   muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = right_spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "   much   acho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = no_space_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(left_spaced_str);
  AR_EXP_Free(right_spaced_str);
  AR_EXP_Free(spaced_str);
  AR_EXP_Free(no_space_str);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, SubstringTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("SUBSTRING", 3);
  AR_ExpNode *original_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *start = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
  AR_ExpNode *length = AR_EXP_NewConstOperandNode(SI_DoubleVal(4));
  AR_ExpNode *start_middle = AR_EXP_NewConstOperandNode(SI_DoubleVal(3));
  AR_ExpNode *length_overflow = AR_EXP_NewConstOperandNode(SI_DoubleVal(20));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = original_str;
  root->op.children[1] = start;
  root->op.children[2] = length;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "much";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = original_str;
  root->op.children[1] = start_middle;
  root->op.children[2] = length_overflow;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "hacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  root->op.children[1] = start_middle;
  root->op.children[2] = length_overflow;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(original_str);
  AR_EXP_Free(start);
  AR_EXP_Free(length);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, ToLowerTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("toLower", 1);
  AR_ExpNode *str1 = AR_EXP_NewConstOperandNode(SI_StringVal("MuChAcHo"));
  AR_ExpNode *str2 = AR_EXP_NewConstOperandNode(SI_StringVal("mUcHaChO"));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str1;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = str2;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(str1);
  AR_EXP_Free(str2);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, ToUpperTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("toUpper", 1);
  AR_ExpNode *str1 = AR_EXP_NewConstOperandNode(SI_StringVal("MuChAcHo"));
  AR_ExpNode *str2 = AR_EXP_NewConstOperandNode(SI_StringVal("mUcHaChO"));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str1;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "MUCHACHO";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = str2;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "MUCHACHO";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(str1);
  AR_EXP_Free(str2);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, ToStringTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("toString", 1);
  AR_ExpNode *str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *number = AR_EXP_NewConstOperandNode(SI_DoubleVal(3.14));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = str;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = number;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "3.140000";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(str);
  AR_EXP_Free(number);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, TrimTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("trim", 1);
  AR_ExpNode *left_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   muchacho"));
  AR_ExpNode *right_spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho   "));
  AR_ExpNode *spaced_str = AR_EXP_NewConstOperandNode(SI_StringVal("   much   acho   "));
  AR_ExpNode *no_space_str = AR_EXP_NewConstOperandNode(SI_StringVal("muchacho"));
  AR_ExpNode *null = AR_EXP_NewConstOperandNode(SI_NullVal());

  root->op.children[0] = left_spaced_str;
  SIValue result = AR_EXP_Evaluate(root, emptyRecord);
  const char *expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = right_spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = spaced_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "much   acho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = no_space_str;
  result = AR_EXP_Evaluate(root, emptyRecord);
  expected = "muchacho";
  EXPECT_STREQ(result.stringval, expected);

  root->op.children[0] = null;
  result = AR_EXP_Evaluate(root, emptyRecord);
  EXPECT_EQ(result.type, T_NULL);

  AR_EXP_Free(left_spaced_str );
  AR_EXP_Free(right_spaced_str);
  AR_EXP_Free(spaced_str);
  AR_EXP_Free(no_space_str);
  AR_EXP_Free(root);
}

TEST_F(ArithmeticTest, IDTest) {
  AR_ExpNode *root = AR_EXP_NewOpNode("id", 1);

  Node *node = Node_New(12345, "person", "Joe");
  AR_ExpNode *person_with_id = AR_EXP_NewVariableOperandNode(NULL, "Joe");

  Record r = Record_Empty();
  Record_AddEntry(&r, node->alias, SI_PtrVal(node));

  root->op.children[0] = person_with_id;
  SIValue result = AR_EXP_Evaluate(root, r);
  EXPECT_EQ(result.longval, node->id);

  Node_Free(node);
  AR_EXP_Free(root);
}

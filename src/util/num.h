/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

/**
 * This file contains helpful functions to work with numbers, like
 * checked addition for unsigned integers, among others.
 * To support the "overload" for a single function using different arguments
 * (except for the out parameter), all the arguments for addition are implicitly
 * converted to the max-width type (uint64_t) and only then, inside the body,
 * the checks are performed.
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Checks whether the specified flag is set within the flag variable.
// Evaluates to true if set, to false otherwise.
#define CHECK_FLAG(flag_var, flag_value) \
    ((flag_var & flag_value) == flag_value)

bool checked_add_u8(const uint8_t lhs, const uint8_t rhs, uint8_t *result);
bool checked_add_u16(const uint16_t lhs, const uint16_t rhs, uint16_t *result);
bool checked_add_u32(const uint32_t lhs, const uint32_t rhs, uint32_t *result);
bool checked_add_u64(const uint64_t lhs, const uint64_t rhs, uint64_t *result);

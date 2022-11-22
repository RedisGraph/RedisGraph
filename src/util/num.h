/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
/**
 * This file contains checked addition for unsigned integers.
 * To support "overload" for a single function using different arguments
 * (except for the out parameter), all the arguments for addition are implicitly
 * converted to the max-width type (uint64_t) and only then, inside the body,
 * the checks are performed.
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

bool checked_add_u8(const uint64_t lhs, const uint64_t rhs, uint8_t *result);
bool checked_add_u16(const uint64_t lhs, const uint64_t rhs, uint16_t *result);
bool checked_add_u32(const uint64_t lhs, const uint64_t rhs, uint32_t *result);
bool checked_add_u64(const uint64_t lhs, const uint64_t rhs, uint64_t *result);

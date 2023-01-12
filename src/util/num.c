/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "num.h"

#if __has_builtin(__builtin_add_overflow)
#define CHECKED_ADD(prefix, type) \
bool checked_add_##prefix(const uint64_t lhs, const uint64_t rhs, type *result) { \
return !__builtin_add_overflow(lhs, rhs, result); \
}
#else
#define CHECKED_ADD(prefix, type) \
bool checked_add_##prefix(const uint64_t lhs, const uint64_t rhs, type *result) { \
    if (!result) { \
        return false; \
    } \
\
    const type temp_result = (type)lhs + (type)rhs; \
    if (temp_result < lhs || temp_result < rhs) { \
        return false; \
    } \
\
    *result = (type)temp_result; \
\
    return true; \
}
#endif // __builtin_add_overflow

CHECKED_ADD(u8, uint8_t);
CHECKED_ADD(u16, uint16_t);
CHECKED_ADD(u32, uint32_t);
CHECKED_ADD(u64, uint64_t);

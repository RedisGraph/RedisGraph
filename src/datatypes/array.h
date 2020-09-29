#pragma once

#include "../value.h"

/**
  * @brief  Initialize a new SIValue array type with given capacity
  * @param  initialCapacity:
  * @retval Initialized array
  */
SIValue SIArray_New(u_int32_t initialCapacity);

/**
  * @brief  Appends a new SIValue to a given array
  * @param  siarray: pointer to array
  * @param  value: new value
  */
void SIArray_Append(SIValue *siarray, SIValue value);

/**
  * @brief  Returns a volatile copy of the SIValue from an array in a given index
  * @note   If index is out of bound, SI_NullVal is returned
  *         Caller is expected either to not free the returned value or take ownership on
  *         its own copy
  * @param  siarray: array
  * @param  index: index
  * @retval The value in the requested index
  */
SIValue SIArray_Get(SIValue siarray, u_int32_t index);

/**
  * @brief  Returns the array length
  * @param  siarray:
  * @retval array length
  */
u_int32_t SIArray_Length(SIValue siarray);

/**
  * @brief  Returns a copy of the array
  * @note   The caller needs to free the array
  * @param  siarray:
  * @retval A clone of the given array
  */
SIValue SIArray_Clone(SIValue siarray);

/**
  * @brief  Prints an array into a given buffer
  * @param  list: array to print
  * @param  buf: print buffer (pointer to pointer to allow re allocation)
  * @param  len: print buffer length
  * @param  bytesWritten: the actual number of bytes written to the buffer
  */
void SIArray_ToString(SIValue list, char **buf, size_t *bufferLen, size_t *bytesWritten);

/**
 * @brief  Returns the array hash code.
 * @param  siarray: SIArray.
 * @retval The array hashCode.
 */
XXH64_hash_t SIArray_HashCode(SIValue siarray);

/**
 * @brief  Reverse the order of the array.
 * @param  siarray: SIArray.
 * @retval None
 */
void SIArray_Reverse(SIValue siarray);

/**
  * @brief  delete an array
  * @param  siarray:
  * @retval None
  */
void SIArray_Free(SIValue siarray);

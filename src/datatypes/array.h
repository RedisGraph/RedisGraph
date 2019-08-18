#pragma once

#include "../value.h"

/**
  * @brief  Initialize a new SIValue array type with given capacity 
  * @param  initialCapacity: 
  * @retval Initialized array
  */
SIValue Array_New(u_int32_t initialCapacity);

/**
  * @brief  Appends a new SIValue to a given array  
  * @param  siarray: array
  * @param  value: new value
  * @retval array with the appended value
  */
SIValue Array_Append(SIValue siarray, SIValue value);

/**
  * @brief  Returns a volatile copy of the SIValue from an array in a given index
  * @note   If index is out of bound, SI_NullVal is returned
  *         Caller is expected either to not free the returned value or take ownership on 
  *         its own copy
  * @param  siarray: array
  * @param  index: index
  * @retval The value in the requested index
  */
SIValue Array_Get(SIValue siarray, u_int32_t index);

/**
  * @brief  Returns the array length  
  * @param  siarray: 
  * @retval array length
  */
u_int32_t Array_Length(SIValue siarray);

/**
  * @brief  Returns a copy of the array  
  * @note   The caller needs to free the array
  * @param  siarray: 
  * @retval 
  */
SIValue Array_Clone(SIValue siarray);

/**
  * @brief  Prints an array into a given buffer
  * @note   If the buffer length is smaller then 6 (for "[...]\0" string) nothing will be printed
  *         If the buffer lenfth is smaller then the overall string length, the string will be truncated and will be 
  *         finished with "...]\0"
  * @param  list: array to print
  * @param  *buf: print buffer
  * @param  len: print buffer length
  * @retval The printed string length
  */
int Array_ToString(SIValue list, char *buf, size_t len);

/**
  * @brief  delete an array  
  * @param  siarray: 
  * @retval None
  */
void Array_Free(SIValue siarray);

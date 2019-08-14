#pragma once

#include "../value.h"

/**
  * @brief  Initialize a new SIValue array with a given capacity
  * @note   
  * @param  initialCapacity: 
  * @retval Initialized array
  */
SIValue Array_New(u_int64_t initialCapacity);

/**
  * @brief  Appends a new SIValue to a given array
  * @note   
  * @param  siarray: array
  * @param  value: new value
  * @retval array with the appended value
  */
SIValue Array_Append(SIValue siarray, SIValue value);

/**
  * @brief  Returns an SIValue from an array in a given index
  * @note   If index is out of bound, SI_NullVal is returned
  * @param  siarray: array
  * @param  index: index
  * @retval The value in the requested index
  */
SIValue Array_Get(SIValue siarray, u_int64_t index);

/**
  * @brief  Returns the array length
  * @note   
  * @param  siarray: 
  * @retval array length
  */
u_int32_t Array_Length(SIValue siarray);

/**
  * @brief  Returns a safe copy of the array
  * @note   
  * @param  siarray: 
  * @retval 
  */
// TODO: remove from here to SIValue clone once our framework support native ref counter
SIValue Array_Clone(SIValue siarray);

/**
  * @brief  Prints an array into the buffer
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
  * @brief  safe delete an array
  * @note   
  * @param  siarray: 
  * @retval None
  */
void Array_Free(SIValue siarray);
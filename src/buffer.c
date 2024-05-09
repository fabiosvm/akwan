//
// buffer.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/buffer.h"
#include <string.h>
#include "akwan/common.h"
#include "akwan/memory.h"

void akw_buffer_init(AkwBuffer *buf)
{
  int capacity = AKW_MIN_CAPACITY;
  uint8_t *bytes = akw_memory_alloc(capacity);
  buf->capacity = capacity;
  buf->count = 0;
  buf->bytes = bytes;
}

void akw_buffer_init_with_capacity(AkwBuffer *buf, int capacity, int *rc)
{
  if (capacity > AKW_MAX_CAPACITY)
  {
    *rc = AKW_RANGE_ERROR;
    return;
  }
  int realCapacity = AKW_MIN_CAPACITY;
  while (realCapacity < capacity)
    realCapacity <<= 1;
  uint8_t *bytes = akw_memory_alloc(realCapacity);
  buf->capacity = realCapacity;
  buf->count = 0;
  buf->bytes = bytes;
}

void akw_buffer_deinit(AkwBuffer *buf)
{
  akw_memory_dealloc(buf->bytes);
}

void akw_buffer_ensure_capacity(AkwBuffer *buf, int capacity, int *rc)
{
  if (capacity <= buf->capacity) return;
  if (capacity > AKW_MAX_CAPACITY)
  {
    *rc = AKW_RANGE_ERROR;
    return;
  }
  int newCapacity = buf->capacity;
  while (newCapacity < capacity)
    newCapacity <<= 1;
  uint8_t *newBytes = akw_memory_realloc(buf->bytes, newCapacity);
  buf->capacity = newCapacity;
  buf->bytes = newBytes;
}

void akw_buffer_write(AkwBuffer *buf, int count, void *ptr, int *rc)
{
  akw_buffer_ensure_capacity(buf, buf->count + count, rc);
  if (!akw_is_ok(*rc)) return;
  memcpy(&buf->bytes[buf->count], ptr, count);
  buf->count += count;
}

//
// string.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/string.h"
#include <stdio.h>
#include <string.h>
#include "akwan/common.h"
#include "akwan/memory.h"

static inline void string_init(AkwString *str, int length, int capacity);

static inline void string_init(AkwString *str, int length, int capacity)
{
  char *chars = akw_memory_alloc(capacity);
  akw_object_init(&str->obj);
  str->capacity = capacity;
  str->length = length;
  str->chars = chars;
}

void akw_string_init(AkwString *str)
{
  string_init(str, 0, AKW_MIN_CAPACITY);
}

void akw_string_init_with_capacity(AkwString *str, int capacity, int *rc)
{
  if (capacity > AKW_MAX_CAPACITY)
  {
    *rc = AKW_RANGE_ERROR;
    return;
  }
  int realCapacity = AKW_MIN_CAPACITY;
  while (realCapacity < capacity)
    realCapacity <<= 1;
  string_init(str, 0, realCapacity);
}

void akw_string_init_from(AkwString *str, int length, char *chars, int *rc)
{
  length = (length < 0) ? (int) strlen(chars) : length;
  if (length > AKW_MAX_CAPACITY)
  {
    *rc = AKW_RANGE_ERROR;
    return;
  }
  int capacity = AKW_MIN_CAPACITY;
  while (capacity < length)
    capacity <<= 1;
  string_init(str, length, capacity);
  memcpy(str->chars, chars, length);
}

void akw_string_deinit(AkwString *str)
{
  akw_memory_dealloc(str->chars);
}

AkwString *akw_string_new(void)
{
  AkwString *str = akw_memory_alloc(sizeof(*str));
  akw_string_init(str);
  return str;
}

AkwString *akw_string_new_with_capacity(int capacity, int *rc)
{
  AkwString *str = akw_memory_alloc(sizeof(*str));
  akw_string_init_with_capacity(str, capacity, rc);
  if (!akw_is_ok(*rc))
  {
    akw_memory_dealloc(str);
    return NULL;
  }
  return str;
}

AkwString *akw_string_new_from(int length, char *chars, int *rc)
{
  AkwString *str = akw_memory_alloc(sizeof(*str));
  akw_string_init_from(str, length, chars, rc);
  if (!akw_is_ok(*rc))
  {
    akw_memory_dealloc(str);
    return NULL;
  }
  return str;
}

void akw_string_free(AkwString *str)
{
  akw_string_deinit(str);
  akw_memory_dealloc(str);
}

void akw_string_release(AkwString *str)
{
  AkwObject *obj = &str->obj;
  --obj->refCount;
  if (obj->refCount) return;
  akw_string_free(str);
}

void akw_string_ensure_capacity(AkwString *str, int capacity, int *rc)
{
  if (capacity <= str->capacity) return;
  if (capacity > AKW_MAX_CAPACITY)
  {
    *rc = AKW_RANGE_ERROR;
    return;
  }
  int newCapacity = str->capacity;
  while (newCapacity < capacity)
    newCapacity <<= 1;
  char *newChars = akw_memory_realloc(str->chars, newCapacity);
  str->capacity = newCapacity;
  str->chars = newChars;
}

void akw_string_print(AkwString *str, bool quoted)
{
  printf(quoted ? "\"%.*s\"" : "%.*s", str->length, str->chars);
}

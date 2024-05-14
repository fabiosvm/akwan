//
// array.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/array.h"
#include <stdio.h>

void akw_array_init(AkwArray *arr)
{
  akw_object_init(&arr->obj);
  akw_vector_init(&arr->vec);
}

void akw_array_init_with_capacity(AkwArray *arr, int capacity, int *rc)
{
  akw_object_init(&arr->obj);
  akw_vector_init_with_capacity(&arr->vec, capacity, rc);
}

void akw_array_deinit(AkwArray *arr)
{
  int n = akw_array_count(arr);
  for (int i = 0; i < n; ++i)
  {
    AkwValue elem = akw_array_get(arr, i);
    akw_value_release(elem);
  }
  akw_vector_deinit(&arr->vec);
}

AkwArray *akw_array_new(void)
{
  AkwArray *arr = akw_memory_alloc(sizeof(*arr));
  akw_array_init(arr);
  return arr;
}

AkwArray *akw_array_new_with_capacity(int capacity, int *rc)
{
  AkwArray *arr = akw_memory_alloc(sizeof(*arr));
  akw_array_init_with_capacity(arr, capacity, rc);
  if (!akw_is_ok(*rc))
  {
    akw_memory_dealloc(arr);
    return NULL;
  }
  return arr;
}

void akw_array_free(AkwArray *arr)
{
  akw_array_deinit(arr);
  akw_memory_dealloc(arr);
}

void akw_array_release(AkwArray *arr)
{
  AkwObject *obj = &arr->obj;
  --obj->refCount;
  if (obj->refCount) return;
  akw_array_free(arr);
}

void akw_array_ensure_capacity(AkwArray *arr, int capacity, int *rc)
{
  akw_vector_ensure_capacity(&arr->vec, capacity, rc);
}

void akw_array_print(AkwArray *arr)
{
  printf("[");
  int n = akw_array_count(arr);
  for (int i = 0; i < n; ++i)
  {
    AkwValue elem = akw_array_get(arr, i);
    akw_value_print(elem, true);
    if (i < n - 1) printf(", ");
  }
  printf("]");
}

void akw_array_inplace_append(AkwArray *arr, AkwValue elem, int *rc)
{
  akw_vector_append(&arr->vec, elem, rc);
  if (!akw_is_ok(*rc)) return;
  akw_value_retain(elem);
}

void akw_array_inplace_set(AkwArray *arr, int index, AkwValue elem)
{
  akw_value_retain(elem);
  akw_value_release(akw_array_get(arr, index));
  akw_vector_set(&arr->vec, index, elem);
}

void akw_array_inplace_remove_at(AkwArray *arr, int index)
{
  AkwValue elem = akw_array_get(arr, index);
  akw_vector_remove_at(&arr->vec, index);
  akw_value_release(elem);
}

void akw_array_inplace_clear(AkwArray *arr)
{
  int n = akw_array_count(arr);
  for (int i = 0; i < n; ++i)
  {
    AkwValue elem = akw_array_get(arr, i);
    akw_value_release(elem);
  }
  akw_vector_clear(&arr->vec);
}

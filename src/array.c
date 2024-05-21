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
#include <assert.h>
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
    AkwValue val = akw_array_get(arr, i);
    akw_value_release(val);
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
    AkwValue val = akw_array_get(arr, i);
    akw_value_print(val, true);
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
  AkwValue val = akw_array_get(arr, index);
  akw_vector_remove_at(&arr->vec, index);
  akw_value_release(val);
}

void akw_array_inplace_concat(AkwArray *arr, AkwArray *other, int *rc)
{
  if (akw_array_is_empty(other)) return;
  int n = akw_array_count(arr);
  int m = akw_array_count(other);
  akw_vector_ensure_capacity(&arr->vec, n + m, rc);
  if (!akw_is_ok(*rc)) return;
  for (int i = 0; i < m; ++i)
  {
    AkwValue val = akw_array_get(other, i);
    akw_array_inplace_append(arr, val, rc);
    if (!akw_is_ok(*rc)) return;
  }
}

void akw_array_clear(AkwArray *arr)
{
  int n = akw_array_count(arr);
  for (int i = 0; i < n; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_value_release(val);
  }
  akw_vector_clear(&arr->vec);
}

AkwArray *akw_array_copy(AkwArray *arr)
{
  int n = akw_array_count(arr);
  int rc = AKW_OK;
  AkwArray *result = akw_array_new_with_capacity(n, &rc);
  assert(akw_is_ok(rc));
  for (int i = 0; i < n; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i, val);
    akw_value_retain(val);
  }
  result->vec.count = n;
  return result;
}

AkwArray *akw_array_append(AkwArray *arr, AkwValue elem, int *rc)
{
  int n = akw_array_count(arr);
  AkwArray *result = akw_array_new_with_capacity(n + 1, rc);
  if (!akw_is_ok(*rc)) return NULL;
  for (int i = 0; i < n; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i, val);
    akw_value_retain(val);
  }
  akw_vector_set(&result->vec, n, elem);
  akw_value_retain(elem);
  result->vec.count = n + 1;
  return result;
}

AkwArray *akw_array_set(AkwArray *arr, int index, AkwValue elem)
{
  int n = akw_array_count(arr);
  int rc = AKW_OK;
  AkwArray *result = akw_array_new_with_capacity(n, &rc);
  assert(akw_is_ok(rc));
  for (int i = 0; i < index; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i, val);
    akw_value_retain(val);
  }
  akw_vector_set(&result->vec, index, elem);
  akw_value_retain(elem);
  for (int i = index + 1; i < n; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i, val);
    akw_value_retain(val);
  }
  result->vec.count = n;
  return result;
}

AkwArray *akw_array_remove_at(AkwArray *arr, int index)
{
  int n = akw_array_count(arr);
  int rc = AKW_OK;
  AkwArray *result = akw_array_new_with_capacity(n - 1, &rc);
  assert(akw_is_ok(rc));
  for (int i = 0; i < index; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i, val);
    akw_value_retain(val);
  }
  for (int i = index + 1; i < n; ++i)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, i - 1, val);
    akw_value_retain(val);
  }
  AkwValue val = akw_array_get(arr, index);
  akw_value_release(val);
  result->vec.count = n - 1;
  return result;
}

AkwArray *akw_array_concat(AkwArray *arr, AkwArray *other, int *rc)
{
  int n = akw_array_count(arr);
  int m = akw_array_count(other);
  AkwArray *result = akw_array_new_with_capacity(n + m, rc);
  if (!akw_is_ok(*rc)) return NULL;
  int j = 0;
  for (int i = 0; i < n; ++i, ++j)
  {
    AkwValue val = akw_array_get(arr, i);
    akw_vector_set(&result->vec, j, val);
    akw_value_retain(val);
  }
  for (int i = 0; i < m; ++i, ++j)
  {
    AkwValue val = akw_array_get(other, i);
    akw_vector_set(&result->vec, j, val);
    akw_value_retain(val);
  }
  result->vec.count = n + m;
  return result;
}

//
// array.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_ARRAY_H
#define AKW_ARRAY_H

#include "value.h"
#include "vector.h"

#define akw_array_count(a)    ((a)->vec.count)
#define akw_array_is_empty(a) (!akw_array_count(a))
#define akw_array_get(a, i)   (akw_vector_get(&(a)->vec, i))

typedef struct
{
  AkwObject           obj;
  AkwVector(AkwValue) vec;
} AkwArray;

void akw_array_init(AkwArray *arr);
void akw_array_init_with_capacity(AkwArray *arr, int capacity, int *rc);
void akw_array_deinit(AkwArray *arr);
AkwArray *akw_array_new(void);
AkwArray *akw_array_new_with_capacity(int capacity, int *rc);
void akw_array_free(AkwArray *arr);
void akw_array_release(AkwArray *arr);
void akw_array_ensure_capacity(AkwArray *arr, int capacity, int *rc);
void akw_array_print(AkwArray *arr);
void akw_array_inplace_append(AkwArray *arr, AkwValue elem, int *rc);
void akw_array_inplace_set(AkwArray *arr, int index, AkwValue elem);
void akw_array_inplace_remove_at(AkwArray *arr, int index);
void akw_array_inplace_concat(AkwArray *arr, AkwArray *other, int *rc);
void akw_array_clear(AkwArray *arr);
AkwArray *akw_array_copy(AkwArray *arr);
AkwArray *akw_array_append(AkwArray *arr, AkwValue elem, int *rc);
AkwArray *akw_array_set(AkwArray *arr, int index, AkwValue elem);
AkwArray *akw_array_remove_at(AkwArray *arr, int index);
AkwArray *akw_array_concat(AkwArray *arr, AkwArray *other, int *rc);

#endif // AKW_ARRAY_H

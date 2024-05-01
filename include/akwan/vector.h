//
// vector.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_VECTOR_H
#define AKW_VECTOR_H

#include "common.h"
#include "memory.h"

#define AkwVector(T) \
  struct { \
    int capacity; \
    int count; \
    T   *elements; \
  }

#define akw_vector_init(v) \
  do { \
    int capacity = AKW_MIN_CAPACITY; \
    size_t size = sizeof(*(v)->elements) * capacity; \
    void *elements = akw_memory_alloc(size); \
    (v)->capacity = capacity; \
    (v)->count = 0; \
    (v)->elements = elements; \
  } while (0)

#define akw_vector_deinit(v) \
  do { \
    akw_memory_dealloc((v)->elements); \
  } while (0)

#define akw_vector_is_empty(v) (!(v)->count)

#define akw_vector_get(v, i) ((v)->elements[(i)])

#define akw_vector_append(v, e, rc) \
  do { \
    int capacity = (v)->capacity; \
    if ((v)->count == capacity) { \
      if (capacity == AKW_MAX_CAPACITY) { \
        *(rc) = AKW_ERR_MAX_CAPACITY_EXCEEDED; \
        break; \
      } \
      int newCapacity = capacity << 1; \
      size_t newSize = sizeof(*(v)->elements) * newCapacity; \
      void *elements = akw_memory_realloc((v)->elements, newSize); \
      (v)->capacity = newCapacity; \
      (v)->elements = elements; \
    } \
    (v)->elements[(v)->count] = (e); \
    ++(v)->count; \
  } while (0)

#define akw_vector_set(v, i, e) \
  do { \
    (v)->elements[(i)] = (e); \
  } while (0)

#define akw_vector_remove_at(v, i) \
  do { \
    --(v)->count; \
    for (int j = (i); j < (v)->count; ++j) \
      (v)->elements[j] = (v)->elements[j + 1]; \
  } while (0)

#define akw_vector_clear(v) \
  do { \
    (v)->count = 0; \
  } while (0)

#endif // AKW_VECTOR_H

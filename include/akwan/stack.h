//
// stack.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_STACK_H
#define AKW_STACK_H

#include "memory.h"

#define AkwStack(T) \
  struct { \
    int size; \
    T   *elements; \
    T   *top; \
    T   *bottom; \
  }

#define akw_stack_init(stk, sz) \
  do { \
    size_t size = sizeof(*(stk)->elements) * (sz); \
    void *elements = akw_memory_alloc(size); \
    (stk)->size = (sz); \
    (stk)->elements = elements; \
    (stk)->top = &(stk)->elements[-1]; \
    (stk)->bottom = &(stk)->elements[(sz) - 1]; \
  } while (0)

#define akw_stack_deinit(stk) \
  do { \
    akw_memory_dealloc((stk)->elements); \
  } while (0)

#define akw_stack_is_empty(stk) ((stk)->top < (stk)->elements)

#define akw_stack_is_full(stk) ((stk)->top == (stk)->bottom)

#define akw_stack_get(stk, i) ((stk)->top[-(i)])

#define akw_stack_set(stk, i, e) \
  do { \
    (stk)->top[-(i)] = (e); \
  } while (0)

#define akw_stack_push(stk, e) \
  do { \
    ++(stk)->top; \
    (stk)->top[0] = (e); \
  } while (0)

#define akw_stack_pop(stk) \
  do { \
    --(stk)->top; \
  } while (0)

#endif // AKW_STACK_H

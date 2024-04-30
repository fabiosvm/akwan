//
// memory.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/memory.h"
#include <stdlib.h>

void *akw_memory_alloc(size_t size)
{
  return malloc(size);
}

void *akw_memory_realloc(void *ptr, size_t newSize)
{
  return realloc(ptr, newSize);
}

void akw_memory_dealloc(void *ptr)
{
  free(ptr);
}

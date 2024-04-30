//
// memory.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_MEMORY_H
#define AKW_MEMORY_H

#include <stddef.h>

void *akw_memory_alloc(size_t size);
void *akw_memory_realloc(void *ptr, size_t newSize);
void akw_memory_dealloc(void *ptr);

#endif // AKW_MEMORY_H

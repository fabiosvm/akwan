//
// buffer.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_BUFFER_H
#define AKW_BUFFER_H

#include <stdint.h>

#define akw_buffer_is_empty(b) (!(b)->count)

#define akw_buffer_inplace_clear(b) \
  do { \
    (b)->count = 0; \
  } while (0)

typedef struct
{
  int     capacity;
  int     count;
  uint8_t *bytes;
} AkwBuffer;

void akw_buffer_init(AkwBuffer *buf);
void akw_buffer_init_with_capacity(AkwBuffer *buf, int capacity, int *rc);
void akw_buffer_deinit(AkwBuffer *buf);
void akw_buffer_ensure_capacity(AkwBuffer *buf, int capacity, int *rc);
void akw_buffer_write(AkwBuffer *buf, void *ptr, int count, int *rc);

#endif // AKW_BUFFER_H

//
// range.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_RANGE_H
#define AKW_RANGE_H

#include <stdint.h>
#include "value.h"

#define akw_range_count(r)  ((r)->start < (r)->end ? (r)->end - (r)->start : 0)
#define akw_range_get(r, i) ((r)->start + (i))

typedef struct
{
  AkwObject obj;
  int64_t   start;
  int64_t   end;
} AkwRange;

void akw_range_init(AkwRange *range, int64_t start, int64_t end);
AkwRange *akw_range_new(int64_t start, int64_t end);
void akw_range_free(AkwRange *range);
void akw_range_release(AkwRange *range);
void akw_range_print(AkwRange *range);

#endif // AKW_RANGE_H

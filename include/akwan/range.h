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

typedef struct
{
  AkwObject obj;
  int64_t   start;
  int64_t   finish;
} AkwRange;

void akw_range_init(AkwRange *range, int64_t start, int64_t finish);
AkwRange *akw_range_new(int64_t start, int64_t finish);
void akw_range_free(AkwRange *range);
void akw_range_release(AkwRange *range);
void akw_range_print(AkwRange *range);

#endif // AKW_RANGE_H

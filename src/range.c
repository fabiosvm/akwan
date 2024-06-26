//
// range.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/range.h"
#include <stdio.h>
#include "akwan/memory.h"

void akw_range_init(AkwRange *range, int64_t start, int64_t end)
{
  akw_object_init(&range->obj);
  range->start = start;
  range->end = end;
}

AkwRange *akw_range_new(int64_t start, int64_t end)
{
  AkwRange *range = akw_memory_alloc(sizeof(*range));
  akw_range_init(range, start, end);
  return range;
}

void akw_range_free(AkwRange *range)
{
  akw_memory_dealloc(range);
}

void akw_range_release(AkwRange *range)
{
  AkwObject *obj = &range->obj;
  --obj->refCount;
  if (obj->refCount) return;
  akw_range_free(range);
}

void akw_range_print(AkwRange *range)
{
  long long start = (long long) range->start;
  long long end = (long long) range->end;
  printf("%lld..%lld", start, end);
}

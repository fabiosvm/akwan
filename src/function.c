//
// function.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/function.h"

AkwFunction *akw_function_new(int arity)
{
  AkwFunction *fn = akw_memory_alloc(sizeof(*fn));
  fn->arity = arity;
  akw_chunk_init(&fn->chunk);
  akw_vector_init(&fn->functions);
  return fn;
}

void akw_function_free(AkwFunction *fn)
{
  akw_chunk_deinit(&fn->chunk);
  int n = fn->functions.count;
  for (int i = 0; i < n; ++i)
  {
    AkwFunction *child = akw_vector_get(&fn->functions, i);
    akw_function_release(child);
  }
  akw_vector_deinit(&fn->functions);
  akw_memory_dealloc(fn);
}

void akw_function_release(AkwFunction *fn)
{
  AkwObject *obj = &fn->obj;
  --obj->refCount;
  if (obj->refCount) return;
  akw_function_free(fn);
}

int akw_function_append_function(AkwFunction *fn, AkwFunction *child, int *rc)
{
  int index = fn->functions.count;
  akw_vector_append(&fn->functions, child, rc);
  if (!akw_is_ok(*rc)) return 0;
  akw_object_retain(&child->obj);
  return index;
}

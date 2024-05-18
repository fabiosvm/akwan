//
// native.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/native.h"
#include "akwan/memory.h"

void akw_native_init(AkwNative *native, int arity, AkwNativeCallFn call)
{
  akw_object_init(&native->obj);
  native->arity = arity;
  native->call = call;
}

AkwNative *akw_native_new(int arity, AkwNativeCallFn call)
{
  AkwNative *native = akw_memory_alloc(sizeof(*native));
  akw_native_init(native, arity, call);
  return native;
}

void akw_native_free(AkwNative *native)
{
  akw_memory_dealloc(native);
}

void akw_native_release(AkwNative *native)
{
  AkwObject *obj = &native->obj;
  --obj->refCount;
  if (obj->refCount) return;
  akw_native_free(native);
}

//
// native.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_NATIVE_H
#define AKW_NATIVE_H

#include "value.h"

struct AkwVM;

typedef void (*AkwNativeCallFn)(struct AkwVM *, AkwValue *);

typedef struct
{
  AkwObject       obj;
  int             arity;
  AkwNativeCallFn call;
} AkwNative;

void akw_native_init(AkwNative *native, int arity, AkwNativeCallFn call);
AkwNative *akw_native_new(int arity, AkwNativeCallFn call);
void akw_native_free(AkwNative *native);
void akw_native_release(AkwNative *native);

#endif // AKW_NATIVE_H

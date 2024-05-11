//
// function.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_FUNCTION_H
#define AKW_FUNCTION_H

#include "chunk.h"

typedef struct AkwFunction
{
  AkwObject                       obj;
  int                             arity;
  AkwChunk                        chunk;
  AkwVector(struct AkwFunction *) functions;
} AkwFunction;

AkwFunction *akw_function_new(int arity);
void akw_function_free(AkwFunction *fn);
void akw_function_release(AkwFunction *fn);
int akw_function_append_function(AkwFunction *fn, AkwFunction *child, int *rc);

#endif // AKW_FUNCTION_H

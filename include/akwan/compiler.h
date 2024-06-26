//
// compiler.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_COMPILER_H
#define AKW_COMPILER_H

#include "chunk.h"
#include "lexer.h"

#define AKW_COMPILER_FLAG_CHECK_ONLY (1 << 0)

#define akw_compiler_is_ok(c) (akw_is_ok((c)->rc))

#define akw_type_info(r) ((AkwTypeInfo) { .isRef = (r) })

typedef struct
{
  bool isRef;
} AkwTypeInfo;

typedef struct
{
  AkwToken    name;
  int         depth;
  AkwTypeInfo typeInfo;
  uint8_t     index;
} AkwVariable;

typedef struct
{
  int                    flags;
  int                    rc;
  AkwError               err;
  AkwLexer               lex;
  int                    scopeDepth;
  AkwVector(AkwVariable) variables;
  AkwTypeInfo            typeInfo;
  AkwChunk               chunk;
} AkwCompiler;

void akw_compiler_init(AkwCompiler *comp, int flags, char *source);
void akw_compiler_deinit(AkwCompiler *comp);
void akw_compiler_compile(AkwCompiler *comp);

#endif // AKW_COMPILER_H

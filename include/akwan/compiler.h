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

typedef enum
{
  AKW_DATA_TYPE_ANY,
  AKW_DATA_TYPE_NIL,
  AKW_DATA_TYPE_BOOL,
  AKW_DATA_TYPE_NUMBER,
  AKW_DATA_TYPE_INT,
  AKW_DATA_TYPE_STRING,
  AKW_DATA_TYPE_RANGE,
  AKW_DATA_TYPE_ARRAY
} AkwDataType;

typedef struct
{
  AkwToken    name;
  int         depth;
  AkwDataType dataType;
  uint8_t     index;
} AkwSymbol;

typedef struct
{
  int                  flags;
  int                  rc;
  AkwError             err;
  AkwLexer             lex;
  int                  scopeDepth;
  AkwVector(AkwSymbol) symbols;
  AkwChunk             chunk;
} AkwCompiler;

const char *akw_data_type_name(AkwDataType dataType);
void akw_compiler_init(AkwCompiler *comp, int flags, char *source);
void akw_compiler_deinit(AkwCompiler *comp);
void akw_compiler_compile(AkwCompiler *comp);

#endif // AKW_COMPILER_H

//
// chunk.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_CHUNK_H
#define AKW_CHUNK_H

#include "buffer.h"
#include "value.h"
#include "vector.h"

typedef enum
{
  AKW_OP_NIL,   AKW_OP_FALSE, AKW_OP_TRUE,
  AKW_OP_CONST, AKW_OP_LOAD,  AKW_OP_STORE,
  AKW_OP_POP,   AKW_OP_ADD,   AKW_OP_SUB,
  AKW_OP_MUL,   AKW_OP_DIV,   AKW_OP_MOD,
  AKW_OP_NEG,   AKW_OP_RETURN
} AkwOpcode;

typedef struct
{
  AkwBuffer           code;
  AkwVector(AkwValue) consts;
} AkwChunk;

const char *akw_opcode_name(AkwOpcode op);
void akw_chunk_init(AkwChunk *chunk);
void akw_chunk_deinit(AkwChunk *chunk);
void akw_chunk_emit_opcode(AkwChunk *chunk, AkwOpcode op, int *rc);
void akw_chunk_emit_byte(AkwChunk *chunk, uint8_t byte, int *rc);
int akw_chunk_append_constant(AkwChunk *chunk, AkwValue val, int *rc);

#endif // AKW_CHUNK_H

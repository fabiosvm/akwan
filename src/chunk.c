//
// chunk.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/chunk.h"

const char *akw_opcode_name(AkwOpcode op)
{
  char *name = "Nil";
  switch (op)
  {
  case AKW_OP_NIL:
    break;
  case AKW_OP_FALSE:
    name = "False";
    break;
  case AKW_OP_TRUE:
    name = "True";
    break;
  case AKW_OP_CONST:
    name = "Const";
    break;
  case AKW_OP_RANGE:
    name = "Range";
    break;
  case AKW_OP_ARRAY:
    name = "Array";
    break;
  case AKW_OP_LOAD:
    name = "Load";
    break;
  case AKW_OP_STORE:
    name = "Store";
    break;
  case AKW_OP_POP:
    name = "Pop";
    break;
  case AKW_OP_INDEX:
    name = "Index";
    break;
  case AKW_OP_ADD:
    name = "Add";
    break;
  case AKW_OP_SUB:
    name = "Sub";
    break;
  case AKW_OP_MUL:
    name = "Mul";
    break;
  case AKW_OP_DIV:
    name = "Div";
    break;
  case AKW_OP_MOD:
    name = "Mod";
    break;
  case AKW_OP_NEG:
    name = "Neg";
    break;
  case AKW_OP_RETURN:
    name = "Return";
    break;
  }
  return name;
}

void akw_chunk_init(AkwChunk *chunk)
{
  akw_buffer_init(&chunk->code);
  akw_vector_init(&chunk->consts);
}

void akw_chunk_deinit(AkwChunk *chunk)
{
  akw_buffer_deinit(&chunk->code);
  int n = chunk->consts.count;
  for (int i = 0; i < n; ++i)
  {
    AkwValue val = akw_vector_get(&chunk->consts, i);
    akw_value_release(val);
  }
  akw_vector_deinit(&chunk->consts);
}

void akw_chunk_emit_opcode(AkwChunk *chunk, AkwOpcode op, int *rc)
{
  akw_buffer_write(&chunk->code, sizeof(uint8_t), &op,  rc);
}

void akw_chunk_emit_byte(AkwChunk *chunk, uint8_t byte, int *rc)
{
  akw_buffer_write(&chunk->code, sizeof(byte), &byte, rc);
}

int akw_chunk_append_constant(AkwChunk *chunk, AkwValue val, int *rc)
{
  int index = chunk->consts.count;
  akw_vector_append(&chunk->consts, val, rc);
  if (!akw_is_ok(*rc)) return 0;
  akw_value_retain(val);
  return index;
}

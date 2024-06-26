//
// dump.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/dump.h"
#include <stdio.h>

void akw_dump_chunk(const AkwChunk *chunk)
{
  printf("; chunk %p\n", (void *) chunk);
  printf("; %d constant(s)\n", chunk->consts.count);
  uint8_t *code = chunk->code.bytes;
  int n = chunk->code.count;
  int j = 0;
  for (int i = 0; i < n;)
  {
    AkwOpcode op = (AkwOpcode) code[i];
    printf("[%04x] ", i);
    switch (op)
    {
    case AKW_OP_NIL:
    case AKW_OP_FALSE:
    case AKW_OP_TRUE:
    case AKW_OP_RANGE:
    case AKW_OP_POP:
    case AKW_OP_GET_ELEMENT:
    case AKW_OP_ADD:
    case AKW_OP_SUB:
    case AKW_OP_MUL:
    case AKW_OP_DIV:
    case AKW_OP_MOD:
    case AKW_OP_NEG:
    case AKW_OP_RETURN:
      printf("%-15s\n", akw_opcode_name(op));
      ++i;
      break;
    case AKW_OP_INT:
    case AKW_OP_CONST:
    case AKW_OP_ARRAY:
    case AKW_OP_LOCAL_REF:
    case AKW_OP_GET_LOCAL:
    case AKW_OP_SET_LOCAL:
    case AKW_OP_GET_LOCAL_BY_REF:
    case AKW_OP_SET_LOCAL_BY_REF:
      {
        uint8_t arg = code[i + 1];
        printf("%-15s %-5d\n", akw_opcode_name(op), arg);
        i += 2;
      }
      break;
    }
    ++j;
  }
  printf("; %d instruction(s)\n", j);
  printf("\n");
}

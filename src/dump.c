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

static inline void print_fmt0(uint32_t instr);
static inline void print_fmt1(uint32_t instr);
static inline void print_fmt2(uint32_t instr);
static inline void print_fmt3(uint32_t instr);

static inline void print_fmt0(uint32_t instr)
{
  AkwOpcode op = (AkwOpcode) akw_instr_op(instr);
  printf("%-8s\n", akw_opcode_name(op));
}

static inline void print_fmt1(uint32_t instr)
{
  AkwOpcode op = (AkwOpcode) akw_instr_op(instr);
  uint8_t a = akw_instr_a(instr);
  printf("%-8s %-5d\n", akw_opcode_name(op), a);
}

static inline void print_fmt2(uint32_t instr)
{
  AkwOpcode op = (AkwOpcode) akw_instr_op(instr);
  uint8_t a = akw_instr_a(instr);
  uint8_t b = akw_instr_b(instr);
  printf("%-8s %-5d %-5d\n", akw_opcode_name(op), a, b);
}

static inline void print_fmt3(uint32_t instr)
{
  AkwOpcode op = (AkwOpcode) akw_instr_op(instr);
  uint8_t a = akw_instr_a(instr);
  uint8_t b = akw_instr_b(instr);
  uint8_t c = akw_instr_c(instr);
  printf("%-8s %-5d %-5d %-5d\n", akw_opcode_name(op), a, b, c);
}

void akw_dump_chunk(const AkwChunk *chunk)
{
  int n = chunk->code.count;
  int numConsts = chunk->consts.count;
  int maxSlots = chunk->maxSlots;
  printf("; chunk %p\n", (void *) chunk);
  printf("; %d instruction(s), %d constant(s), %d slot(s)\n", n,
    numConsts, maxSlots);
  uint32_t *code = chunk->code.elements;
  for (int i = 0; i < n; ++i)
  {
    uint32_t instr = code[i];
    AkwOpcode op = (AkwOpcode) akw_instr_op(instr);
    printf("[%04x] ", i);
    switch (op)
    {
    case AKW_OP_RETURN:
      print_fmt0(instr);
      break;
    case AKW_OP_NIL:
    case AKW_OP_FALSE:
    case AKW_OP_TRUE:
      print_fmt1(instr);
      break;
    case AKW_OP_CONST:
    case AKW_OP_MOVE:
    case AKW_OP_NEG:
      print_fmt2(instr);
      break;
    case AKW_OP_ADD:
    case AKW_OP_SUB:
    case AKW_OP_MUL:
    case AKW_OP_DIV:
    case AKW_OP_MOD:
      print_fmt3(instr);
      break;
    }
  }
  printf("\n");
}

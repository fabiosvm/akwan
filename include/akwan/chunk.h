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

#include <stdint.h>
#include "value.h"
#include "vector.h"

#define akw_instr_fmt0(op)          ((uint32_t) (op))
#define akw_instr_fmt1(op, a, bc)   ((uint32_t) (((bc) << 16) | ((a) << 8) | (op)))
#define akw_instr_fmt2(op, a, b)    ((uint32_t) (((b) << 16) | ((a) << 8) | (op)))
#define akw_instr_fmt3(op, a, b, c) ((uint32_t) ((c) << 24) | ((b) << 16) | ((a) << 8) | (op))

#define akw_nil_instr(d)          (akw_instr_fmt1(AKW_OP_NIL, (d), 0))
#define akw_false_instr(d)        (akw_instr_fmt1(AKW_OP_FALSE, (d), 0))
#define akw_true_instr(d)         (akw_instr_fmt1(AKW_OP_TRUE, (d), 0))
#define akw_const_instr(d, index) (akw_instr_fmt2(AKW_OP_CONST, (d), (index)))
#define akw_move_instr(d, s)      (akw_instr_fmt2(AKW_OP_MOVE, (d), (s)))
#define akw_add_instr(d, s1, s2)  (akw_instr_fmt3(AKW_OP_ADD, (d), (s1), (s2)))
#define akw_sub_instr(d, s1, s2)  (akw_instr_fmt3(AKW_OP_SUB, (d), (s1), (s2)))
#define akw_mul_instr(d, s1, s2)  (akw_instr_fmt3(AKW_OP_MUL, (d), (s1), (s2)))
#define akw_div_instr(d, s1, s2)  (akw_instr_fmt3(AKW_OP_DIV, (d), (s1), (s2)))
#define akw_mod_instr(d, s1, s2)  (akw_instr_fmt3(AKW_OP_MOD, (d), (s1), (s2)))
#define akw_neg_instr(d, s)       (akw_instr_fmt2(AKW_OP_NEG, (d), (s)))
#define akw_return_instr()        (akw_instr_fmt0(AKW_OP_RETURN))

#define akw_instr_op(i) ((AkwOpcode) ((i) & 0xff))
#define akw_instr_a(i)  ((uint8_t) (((i) >> 8) & 0xff))
#define akw_instr_b(i)  ((uint8_t) (((i) >> 16) & 0xff))
#define akw_instr_c(i)  ((uint8_t) (((i) >> 24) & 0xff))
#define akw_instr_bc(i) ((uint16_t) ((i) >> 16))

typedef enum
{
  AKW_OP_NIL,  AKW_OP_FALSE, AKW_OP_TRUE, AKW_OP_CONST,
  AKW_OP_MOVE, AKW_OP_ADD,   AKW_OP_SUB,  AKW_OP_MUL,
  AKW_OP_DIV,  AKW_OP_MOD,   AKW_OP_NEG,  AKW_OP_RETURN
} AkwOpcode;

typedef struct
{
  AkwVector(uint32_t) code;
  AkwVector(AkwValue) consts;
  int                 maxSlots;
} AkwChunk;

const char *akw_opcode_name(AkwOpcode op);
void akw_chunk_init(AkwChunk *chunk);
void akw_chunk_deinit(AkwChunk *chunk);
void akw_chunk_emit(AkwChunk *chunk, uint32_t instr, int *rc);
int akw_chunk_append_constant(AkwChunk *chunk, AkwValue val, int *rc);

#endif // AKW_CHUNK_H

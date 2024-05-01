//
// vm.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/vm.h"
#include <math.h>

#define dispatch(vm, c, ip, s) \
  do { \
    AkwOpcode op = (AkwOpcode) akw_instr_op(*(ip)); \
    AkwInstructionHandleFn handle = instructionHandles[op]; \
    handle((vm), (c), (ip), (s)); \
  } while (0)

typedef void (*AkwInstructionHandleFn)(AkwVM *, AkwChunk *, uint32_t *, AkwValue *);

static void do_nil(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_false(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_true(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_const(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_move(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_add(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_sub(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_mul(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_div(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_mod(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_neg(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);
static void do_return(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots);

static AkwInstructionHandleFn instructionHandles[] = {
  [AKW_OP_NIL]   = do_nil,   [AKW_OP_FALSE] = do_false, [AKW_OP_TRUE]   = do_true,
  [AKW_OP_CONST] = do_const, [AKW_OP_MOVE]  = do_move,  [AKW_OP_ADD]    = do_add,
  [AKW_OP_SUB]   = do_sub,   [AKW_OP_MUL]   = do_mul,   [AKW_OP_DIV]    = do_div,
  [AKW_OP_MOD]   = do_mod,   [AKW_OP_NEG]   = do_neg,   [AKW_OP_RETURN] = do_return
};

static void do_nil(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  slots[d] = akw_nil_value();
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_false(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  slots[d] = akw_bool_value(false);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_true(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  slots[d] = akw_bool_value(true);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_const(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t index = akw_instr_b(instr);
  AkwValue *consts = chunk->consts.elements;
  slots[d] = consts[index];
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_move(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s = akw_instr_b(instr);
  slots[d] = slots[s];
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_add(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s1 = akw_instr_b(instr);
  uint8_t s2 = akw_instr_c(instr);
  AkwValue val1 = slots[s1];
  AkwValue val2 = slots[s2];
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot add %s and %s", akw_type_name(akw_type(val1)),
      akw_type_name(akw_type(val2)));
    return;
  }
  double num = akw_as_number(val1) + akw_as_number(val2);
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_sub(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s1 = akw_instr_b(instr);
  uint8_t s2 = akw_instr_c(instr);
  AkwValue val1 = slots[s1];
  AkwValue val2 = slots[s2];
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot subtract %s from %s", akw_type_name(akw_type(val2)),
      akw_type_name(akw_type(val1)));
    return;
  }
  double num = akw_as_number(val1) - akw_as_number(val2);
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_mul(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s1 = akw_instr_b(instr);
  uint8_t s2 = akw_instr_c(instr);
  AkwValue val1 = slots[s1];
  AkwValue val2 = slots[s2];
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot multiply %s by %s", akw_type_name(akw_type(val1)),
      akw_type_name(akw_type(val2)));
    return;
  }
  double num = akw_as_number(val1) * akw_as_number(val2);
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_div(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s1 = akw_instr_b(instr);
  uint8_t s2 = akw_instr_c(instr);
  AkwValue val1 = slots[s1];
  AkwValue val2 = slots[s2];
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot divide %s by %s", akw_type_name(akw_type(val1)),
      akw_type_name(akw_type(val2)));
    return;
  }
  double num = akw_as_number(val1) / akw_as_number(val2);
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_mod(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s1 = akw_instr_b(instr);
  uint8_t s2 = akw_instr_c(instr);
  AkwValue val1 = slots[s1];
  AkwValue val2 = slots[s2];
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot calculate the modulus of %s by %s", akw_type_name(akw_type(val1)),
      akw_type_name(akw_type(val2)));
    return;
  }
  double num = fmod(akw_as_number(val1), akw_as_number(val2));
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_neg(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  uint32_t instr = *ip;
  uint8_t d = akw_instr_a(instr);
  uint8_t s = akw_instr_b(instr);
  AkwValue val = slots[s];
  if (!akw_is_number(val))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot negate %s", akw_type_name(akw_type(val)));
    return;
  }
  double num = - akw_as_number(val);
  slots[d] = akw_number_value(num);
  dispatch(vm, chunk, ip + 1, slots);
}

static void do_return(AkwVM *vm, AkwChunk *chunk, uint32_t *ip, AkwValue *slots)
{
  (void) vm;
  (void) chunk;
  (void) ip;
  (void) slots;
}

void akw_vm_init(AkwVM *vm, int stackSize)
{
  vm->rc = AKW_OK;
  akw_stack_init(&vm->stack, stackSize);
}

void akw_vm_deinit(AkwVM *vm)
{
  akw_stack_deinit(&vm->stack);
}

void akw_vm_run(AkwVM *vm, AkwChunk *chunk)
{
  uint32_t *ip = chunk->code.elements;
  AkwValue *slots = vm->stack.elements;
  dispatch(vm, chunk, ip, slots);
}

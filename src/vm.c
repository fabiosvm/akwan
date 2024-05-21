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
#include <assert.h>
#include <math.h>
#include "akwan/array.h"
#include "akwan/range.h"

#define dispatch(vm, c, ip, s) \
  do { \
    AkwOpcode op = (AkwOpcode) ip[0]; \
    AkwInstructionHandleFn handle = instructionHandles[op]; \
    handle((vm), (c), (ip), (s)); \
  } while (0)

typedef void (*AkwInstructionHandleFn)(AkwVM *, AkwChunk *, uint8_t *, AkwValue *);

static inline void push(AkwVM *vm, AkwValue val);
static void do_nil(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_false(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_true(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_int(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_const(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_range(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_array(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_local_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_pop(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_get_local(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_set_local(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_get_local_by_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_set_local_by_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_get_element(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_add(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_sub(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_mul(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_div(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_mod(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_neg(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);
static void do_return(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots);

static AkwInstructionHandleFn instructionHandles[] = {
  [AKW_OP_NIL]              = do_nil,              [AKW_OP_FALSE]            = do_false,
  [AKW_OP_TRUE]             = do_true,             [AKW_OP_INT]              = do_int,
  [AKW_OP_CONST]            = do_const,            [AKW_OP_RANGE]            = do_range,
  [AKW_OP_ARRAY]            = do_array,            [AKW_OP_LOCAL_REF]        = do_local_ref,
  [AKW_OP_POP]              = do_pop,              [AKW_OP_GET_LOCAL]        = do_get_local,
  [AKW_OP_SET_LOCAL]        = do_set_local,        [AKW_OP_GET_LOCAL_BY_REF] = do_get_local_by_ref,
  [AKW_OP_SET_LOCAL_BY_REF] = do_set_local_by_ref, [AKW_OP_GET_ELEMENT]      = do_get_element,
  [AKW_OP_ADD]              = do_add,              [AKW_OP_SUB]              = do_sub,
  [AKW_OP_MUL]              = do_mul,              [AKW_OP_DIV]              = do_div,
  [AKW_OP_MOD]              = do_mod,              [AKW_OP_NEG]              = do_neg,
  [AKW_OP_RETURN]           = do_return
};

static inline void push(AkwVM *vm, AkwValue val)
{
  if (akw_stack_is_full(&vm->stack))
  {
    vm->rc = AKW_RANGE_ERROR;
    akw_error_set(vm->err, "stack overflow");
    return;
  }
  akw_stack_push(&vm->stack, val);
}

static void do_nil(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  push(vm, akw_nil_value());
  if (!akw_vm_is_ok(vm)) return;
  dispatch(vm, chunk, ip, slots);
}

static void do_false(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  push(vm, akw_bool_value(false));
  if (!akw_vm_is_ok(vm)) return;
  dispatch(vm, chunk, ip, slots);
}

static void do_true(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  push(vm, akw_bool_value(true));
  if (!akw_vm_is_ok(vm)) return;
  dispatch(vm, chunk, ip, slots);
}

static void do_int(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t data = ip[1];
  ip += 2;
  push(vm, akw_int_value(data));
  if (!akw_vm_is_ok(vm)) return;
  dispatch(vm, chunk, ip, slots);
}

static void do_const(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue *consts = chunk->consts.elements;
  AkwValue val = consts[index];
  push(vm, val);
  if (!akw_vm_is_ok(vm)) return;
  akw_value_retain(val);
  dispatch(vm, chunk, ip, slots);
}

static void do_range(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_int(val1) || !akw_is_int(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot create a range with %s and %s",
      akw_value_type_name(val1), akw_value_type_name(val2));
    return;
  }
  int64_t start = akw_as_int(val1);
  int64_t finish = akw_as_int(val2);
  AkwRange *range = akw_range_new(start, finish);
  akw_stack_set(&vm->stack, 1, akw_range_value(range));
  akw_object_retain(&range->obj);
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_array(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t n = ip[1];
  ip += 2;
  AkwValue *_slots = &vm->stack.top[1 - n];
  AkwArray *arr = akw_array_new_with_capacity(n, &vm->rc);
  if (!akw_vm_is_ok(vm))
  {
    assert(vm->rc == AKW_RANGE_ERROR);
    akw_error_set(vm->err, "array too large");
    return;
  }
  arr->vec.count = n;
  for (int i = 0; i < n; ++i)
    akw_vector_set(&arr->vec, i, _slots[i]);
  AkwValue val = akw_array_value(arr);
  _slots[0] = val;
  akw_object_retain(&arr->obj);
  vm->stack.top -= n - 1;
  dispatch(vm, chunk, ip, slots);
}

static void do_local_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue *ref = &slots[index];
  push(vm, akw_ref_value(ref));
  if (!akw_vm_is_ok(vm)) return;
  dispatch(vm, chunk, ip, slots);
}

static void do_pop(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val = akw_stack_get(&vm->stack, 0);
  akw_stack_pop(&vm->stack);
  akw_value_release(val);
  dispatch(vm, chunk, ip, slots);
}

static void do_get_local(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue val = slots[index];
  push(vm, val);
  if (!akw_vm_is_ok(vm)) return;
  akw_value_retain(val);
  dispatch(vm, chunk, ip, slots);
}

static void do_set_local(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue val = akw_stack_get(&vm->stack, 0);
  akw_value_retain(val);
  akw_value_release(slots[index]);
  slots[index] = val;
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_get_local_by_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue *ref = akw_as_ref(slots[index]);
  AkwValue val = *ref;
  push(vm, val);
  if (!akw_vm_is_ok(vm)) return;
  akw_value_retain(val);
  dispatch(vm, chunk, ip, slots);
}

static void do_set_local_by_ref(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  uint8_t index = ip[1];
  ip += 2;
  AkwValue *ref = akw_as_ref(slots[index]);
  AkwValue val = akw_stack_get(&vm->stack, 0);
  akw_value_retain(val);
  akw_value_release(*ref);
  *ref = val;
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_get_element(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_array(val1) || !akw_is_int(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot index %s with %s", akw_value_type_name(val1),
      akw_value_type_name(val2));
    return;
  }
  AkwArray *arr = akw_as_array(val1);
  int64_t index = akw_as_int(val2);
  if (index < 0 || index >= akw_array_count(arr))
  {
    vm->rc = AKW_RANGE_ERROR;
    akw_error_set(vm->err, "index out of range");
    return;
  }
  AkwValue val = akw_array_get(arr, index);
  akw_stack_set(&vm->stack, 1, val);
  akw_value_retain(val);
  akw_array_release(arr);
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_add(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot add %s and %s", akw_value_type_name(val1),
      akw_value_type_name(val2));
    return;
  }
  double num = akw_as_number(val1) + akw_as_number(val2);
  akw_stack_set(&vm->stack, 1, akw_number_value(num));
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_sub(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot subtract %s from %s", akw_value_type_name(val2),
      akw_value_type_name(val1));
    return;
  }
  double num = akw_as_number(val1) - akw_as_number(val2);
  akw_stack_set(&vm->stack, 1, akw_number_value(num));
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_mul(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot multiply %s by %s", akw_value_type_name(val1),
      akw_value_type_name(val2));
    return;
  }
  double num = akw_as_number(val1) * akw_as_number(val2);
  akw_stack_set(&vm->stack, 1, akw_number_value(num));
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_div(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot divide %s by %s", akw_value_type_name(val1),
      akw_value_type_name(val2));
    return;
  }
  double num = akw_as_number(val1) / akw_as_number(val2);
  akw_stack_set(&vm->stack, 1, akw_number_value(num));
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_mod(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val1 = akw_stack_get(&vm->stack, 1);
  AkwValue val2 = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val1) || !akw_is_number(val2))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot calculate the modulus of %s by %s",
      akw_value_type_name(val1), akw_value_type_name(val2));
    return;
  }
  double num = fmod(akw_as_number(val1), akw_as_number(val2));
  akw_stack_set(&vm->stack, 1, akw_number_value(num));
  akw_stack_pop(&vm->stack);
  dispatch(vm, chunk, ip, slots);
}

static void do_neg(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
{
  ++ip;
  AkwValue val = akw_stack_get(&vm->stack, 0);
  if (!akw_is_number(val))
  {
    vm->rc = AKW_TYPE_ERROR;
    akw_error_set(vm->err, "cannot negate %s", akw_value_type_name(val));
    return;
  }
  double num = - akw_as_number(val);
  akw_stack_set(&vm->stack, 0, akw_number_value(num));
  dispatch(vm, chunk, ip, slots);
}

static void do_return(AkwVM *vm, AkwChunk *chunk, uint8_t *ip, AkwValue *slots)
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
  while (!akw_stack_is_empty(&vm->stack))
  {
    AkwValue val = akw_stack_get(&vm->stack, 0);
    akw_stack_pop(&vm->stack);
    akw_value_release(val);
  }
  akw_stack_deinit(&vm->stack);
}

void akw_vm_run(AkwVM *vm, AkwChunk *chunk)
{
  uint8_t *ip = chunk->code.bytes;
  AkwValue *slots = vm->stack.elements;
  dispatch(vm, chunk, ip, slots);
}

void akw_vm_push(AkwVM *vm, AkwValue val)
{
  push(vm, val);
  if (!akw_vm_is_ok(vm)) return;
  akw_value_retain(val);
}

AkwValue akw_vm_peek(AkwVM *vm)
{
  return akw_stack_get(&vm->stack, 0);
}

void akw_vm_pop(AkwVM *vm)
{
  AkwValue val = akw_stack_get(&vm->stack, 0);
  akw_stack_pop(&vm->stack);
  akw_value_release(val);
}

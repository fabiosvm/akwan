//
// vm.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_VM_H
#define AKW_VM_H

#include "error.h"
#include "function.h"
#include "stack.h"

#define AKW_VM_DEFAULT_STACK_SIZE     (1 << 10)
#define AKW_VM_DEFAULT_CALLSTACK_SIZE (1 << 10)

#define akw_vm_is_ok(vm) (akw_is_ok((vm)->rc))

typedef struct
{
  AkwFunction *fn;
  uint8_t     *ip;
  AkwValue    *slots;
} AkwCallFrame;

typedef struct
{
  int                    rc;
  AkwError               err;
  AkwStack(AkwValue)     stack;
  AkwStack(AkwCallFrame) callstack;
} AkwVM;

void akw_vm_init(AkwVM *vm, int stackSize, int cstackSize);
void akw_vm_deinit(AkwVM *vm);
void akw_vm_run(AkwVM *vm, AkwFunction *fn);
void akw_vm_push(AkwVM *vm, AkwValue val);
AkwValue akw_vm_peek(AkwVM *vm);
void akw_vm_pop(AkwVM *vm);

#endif // AKW_VM_H

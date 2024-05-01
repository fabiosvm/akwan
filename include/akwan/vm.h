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

#include "chunk.h"
#include "stack.h"

#define AKW_VM_DEFAULT_STACK_SIZE (1024)

#define akw_vm_is_ok(vm) (akw_is_ok((vm)->rc))

typedef struct
{
  int                rc;
  AkwStack(AkwValue) stack;
} AkwVM;

void akw_vm_init(AkwVM *vm, int stackSize);
void akw_vm_deinit(AkwVM *vm);
void akw_vm_run(AkwVM *vm, AkwChunk *chunk);

#endif // AKW_VM_H
//
// main.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <akwan.h>
#include <stdio.h>
#include <stdlib.h>

static inline void print_error(char *err);

static inline void print_error(char *err)
{
  fprintf(stderr, "ERROR: %s\n", err);
}

int main(void)
{
  char *source = "let x = 10;"
                 "let y = 20;"
                 "return x + y;";
  // Compile
  AkwCompiler comp;
  akw_compiler_init(&comp, source);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(comp.err);
    return EXIT_FAILURE;
  }
  akw_compiler_compile(&comp);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(comp.err);
    akw_compiler_deinit(&comp);
    return EXIT_FAILURE;
  }

  // Dump
  akw_dump_chunk(&comp.chunk);

  // Run
  AkwVM vm;
  akw_vm_init(&vm, AKW_VM_DEFAULT_STACK_SIZE);
  akw_vm_run(&vm, &comp.chunk);
  if (!akw_vm_is_ok(&vm))
  {
    print_error(vm.err);
    akw_compiler_deinit(&comp);
    akw_vm_deinit(&vm);
    return EXIT_FAILURE;
  }

  // Print result
  AkwValue result = akw_stack_get(&vm.stack, 0);
  akw_value_print(result);
  akw_stack_pop(&vm.stack);
  printf("\n");

  // Cleanup
  akw_compiler_deinit(&comp);
  akw_vm_deinit(&vm);
  return EXIT_SUCCESS;
}

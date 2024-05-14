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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static inline void read_from_stdin(AkwBuffer *buf, int *rc);
static inline void print_error(char *err);

static inline void read_from_stdin(AkwBuffer *buf, int *rc)
{
  int c;
  while ((c = getchar()) != EOF)
  {
    akw_buffer_write(buf, 1, &c, rc);
    if (!akw_is_ok(*rc)) return;
  }
  akw_buffer_write(buf, 1, "\0", rc);
}

static inline void print_error(char *err)
{
  fprintf(stderr, "ERROR: %s\n", err);
}

int main(void)
{
  // Read source code
  AkwBuffer buf;
  akw_buffer_init(&buf);
  int rc = AKW_OK;
  read_from_stdin(&buf, &rc);
  if (!akw_is_ok(rc))
  {
    assert(rc == AKW_RANGE_ERROR);
    print_error("source code too large");
    akw_buffer_deinit(&buf);
    return EXIT_FAILURE;
  }

  // Compile
  AkwCompiler comp;
  akw_compiler_init(&comp, 0, (char *) buf.bytes);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(comp.err);
    akw_buffer_deinit(&buf);
    return EXIT_FAILURE;
  }
  akw_compiler_compile(&comp);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(comp.err);
    akw_buffer_deinit(&buf);
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
    akw_buffer_deinit(&buf);
    akw_compiler_deinit(&comp);
    akw_vm_deinit(&vm);
    return EXIT_FAILURE;
  }

  // Print result
  AkwValue result = akw_vm_peek(&vm);
  akw_value_print(result, false);
  akw_vm_pop(&vm);
  printf("\n");

  // Cleanup
  akw_buffer_deinit(&buf);
  akw_compiler_deinit(&comp);
  akw_vm_deinit(&vm);
  return EXIT_SUCCESS;
}

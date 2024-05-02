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

static inline void print_error(AkwCompiler *comp);

static inline void print_error(AkwCompiler *comp)
{
  fprintf(stderr, "ERROR: %s\n", comp->err);
}

int main(void)
{
  char *source = "let x = 10;"
                 "let y = 20;"
                 "return x + y;";

  static AkwCompiler comp;
  akw_compiler_init(&comp, NULL, source);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(&comp);
    return EXIT_FAILURE;
  }

  akw_compiler_compile(&comp);
  if (!akw_compiler_is_ok(&comp))
  {
    print_error(&comp);
    akw_compiler_deinit(&comp);
    return EXIT_FAILURE;
  }

  akw_dump_chunk(&comp.chunk);
  akw_compiler_deinit(&comp);
  return EXIT_SUCCESS;
}

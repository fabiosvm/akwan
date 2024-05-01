//
// error.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/error.h"
#include <stdarg.h>
#include <stdio.h>

void akw_error_set(AkwError err, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(err, AKW_ERROR_MAX_LENGTH + 1, fmt, args);
  va_end(args);
}

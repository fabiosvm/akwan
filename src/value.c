//
// value.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/value.h"
#include <stdio.h>
#include "akwan/range.h"
#include "akwan/string.h"
#include "akwan/function.h"

const char *akw_value_type_name(AkwValue val)
{
  char *name = "Nil";
  switch (akw_type(val))
  {
  case AKW_TYPE_NIL:
    break;
  case AKW_TYPE_BOOL:
    name = "Bool";
    break;
  case AKW_TYPE_NUMBER:
    name = (akw_as_number(val) == akw_as_int(val)) ? "Int" : "Number";
    break;
  case AKW_TYPE_STRING:
    name = "String";
    break;
  case AKW_TYPE_RANGE:
    name = "Range";
    break;
  case AKW_TYPE_FUNCTION:
    name = "Function";
    break;
  }
  return name;
}

void akw_value_free(AkwValue val)
{
  switch (akw_type(val))
  {
  case AKW_TYPE_NIL:
  case AKW_TYPE_BOOL:
  case AKW_TYPE_NUMBER:
    break;
  case AKW_TYPE_STRING:
    akw_string_free(akw_as_string(val));
    break;
  case AKW_TYPE_RANGE:
    akw_range_free(akw_as_range(val));
    break;
  case AKW_TYPE_FUNCTION:
    akw_function_free(akw_as_function(val));
    break;
  }
}

void akw_value_release(AkwValue val)
{
  switch (akw_type(val))
  {
  case AKW_TYPE_NIL:
  case AKW_TYPE_BOOL:
  case AKW_TYPE_NUMBER:
    break;
  case AKW_TYPE_STRING:
    akw_string_release(akw_as_string(val));
    break;
  case AKW_TYPE_RANGE:
    akw_range_release(akw_as_range(val));
    break;
  case AKW_TYPE_FUNCTION:
    akw_function_release(akw_as_function(val));
    break;
  }
}

void akw_value_print(AkwValue val)
{
  switch (akw_type(val))
  {
  case AKW_TYPE_NIL:
    printf("nil");
    break;
  case AKW_TYPE_BOOL:
    printf("%s", akw_as_bool(val) ? "true" : "false");
    break;
  case AKW_TYPE_NUMBER:
    printf("%g", akw_as_number(val));
    break;
  case AKW_TYPE_STRING:
    akw_string_print(akw_as_string(val), false);
    break;
  case AKW_TYPE_RANGE:
    akw_range_print(akw_as_range(val));
    break;
  case AKW_TYPE_FUNCTION:
    printf("<function %p>", val.asPointer);
    break;
  }
}

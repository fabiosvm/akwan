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
#include "akwan/array.h"
#include "akwan/native.h"
#include "akwan/range.h"
#include "akwan/string.h"

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
  case AKW_TYPE_ARRAY:
    name = "Array";
    break;
  case AKW_TYPE_NATIVE:
    name = "Native";
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
  case AKW_TYPE_ARRAY:
    akw_array_free(akw_as_array(val));
    break;
  case AKW_TYPE_NATIVE:
    akw_native_free(akw_as_native(val));
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
  case AKW_TYPE_ARRAY:
    akw_array_release(akw_as_array(val));
    break;
  case AKW_TYPE_NATIVE:
    akw_native_release(akw_as_native(val));
    break;
  }
}

void akw_value_print(AkwValue val, bool quoted)
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
    akw_string_print(akw_as_string(val), quoted);
    break;
  case AKW_TYPE_RANGE:
    akw_range_print(akw_as_range(val));
    break;
  case AKW_TYPE_ARRAY:
    akw_array_print(akw_as_array(val));
    break;
  case AKW_TYPE_NATIVE:
    printf("<native %p>", val.asPointer);
    break;
  }
}

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
#include "akwan/string.h"

const char *akw_type_name(AkwType type)
{
  char *name = "Nil";
  switch (type)
  {
  case AKW_TYPE_NIL:
    break;
  case AKW_TYPE_BOOL:
    name = "Bool";
    break;
  case AKW_TYPE_NUMBER:
    name = "Number";
    break;
  case AKW_TYPE_STRING:
    name = "String";
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
  }
}

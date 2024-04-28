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
  }  
  return name;
}

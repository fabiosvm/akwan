//
// string.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_STRING_H
#define AKW_STRING_H

#include "value.h"

#define akw_string_is_empty(s) (!(s)->count)

typedef struct
{
  AkwObject obj;
  int       capacity;
  int       length;
  char      *chars;
} AkwString;

void akw_string_init(AkwString *str);
void akw_string_init_with_capacity(AkwString *str, int capacity, int *rc);
void akw_string_init_from(AkwString *str, int length, char *chars, int *rc);
void akw_string_deinit(AkwString *str);
AkwString *akw_string_new(void);
AkwString *akw_string_new_with_capacity(int capacity, int *rc);
AkwString *akw_string_new_from(int length, char *chars, int *rc);
void akw_string_free(AkwString *str);
void akw_string_release(AkwString *str);
void akw_string_ensure_capacity(AkwString *str, int capacity, int *rc);
void akw_string_print(AkwString *str, bool quoted);

#endif // AKW_STRING_H

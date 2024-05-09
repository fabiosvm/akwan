//
// value.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_VALUE_H
#define AKW_VALUE_H

#include <stdbool.h>

#define AKW_FALG_FALSY  (0x01)
#define AKW_FLAG_OBJECT (0x02)

#define akw_nil_value()     ((AkwValue) { .type = AKW_TYPE_NIL, .flags = AKW_FALG_FALSY })
#define akw_bool_value(b)   ((AkwValue) { .type = AKW_TYPE_BOOL, .flags = (b) ? 0 : AKW_FALG_FALSY, .asBool = (b) })
#define akw_number_value(n) ((AkwValue) { .type = AKW_TYPE_NUMBER, .flags = 0, .asNumber = (n) })

#define akw_type(v) ((v).type)

#define akw_as_bool(v)   ((v).asBool)
#define akw_as_number(v) ((v).asNumber)
#define akw_as_int(v)    ((int64_t) akw_as_number(v))
#define akw_as_object(v) ((AkwObject *) (v).asPointer)

#define akw_is_nil(v)    (akw_type(v) == AKW_TYPE_NIL)
#define akw_is_bool(v)   (akw_type(v) == AKW_TYPE_BOOL)
#define akw_is_number(v) (akw_type(v) == AKW_TYPE_NUMBER)
#define akw_is_int(v)    (akw_is_number(v) && (akw_as_number(v) == akw_as_int(v)))
#define akw_is_falsy(v)  ((v).flags & AKW_FALG_FALSY)
#define akw_is_object(v) ((v).flags & AKW_FLAG_OBJECT)

#define akw_object_init(o) \
  do { \
    (o)->refCount = 0; \
  } while (0);

#define akw_object_retain(o) \
  do { \
    ++(o)->refCount; \
  } while (0);

#define akw_value_retain(v) \
  do { \
    if (!akw_is_object(v)) break; \
    akw_object_retain(akw_as_object(v)); \
  } while (0);

typedef enum
{
  AKW_TYPE_NIL,
  AKW_TYPE_BOOL,
  AKW_TYPE_NUMBER
} AkwType;

typedef struct
{
  AkwType  type;
  int      flags;
  union
  {
    bool   asBool;
    double asNumber;
    void   *asPointer;
  };
} AkwValue;

typedef struct
{
  int refCount;
} AkwObject;

const char *akw_type_name(AkwType type);
void akw_value_free(AkwValue val);
void akw_value_release(AkwValue val);
void akw_value_print(AkwValue val);

#endif // AKW_VALUE_H

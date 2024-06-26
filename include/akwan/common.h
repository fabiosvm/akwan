//
// common.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_COMMON_H
#define AKW_COMMON_H

#define AKW_OK             (0)
#define AKW_LEXICAL_ERROR  (1)
#define AKW_SYNTAX_ERROR   (2)
#define AKW_SEMANTIC_ERROR (3)
#define AKW_TYPE_ERROR     (4)
#define AKW_RANGE_ERROR    (5)

#define AKW_MIN_CAPACITY (1 << 3)
#define AKW_MAX_CAPACITY (1 << 30)

#define akw_is_ok(rc) ((rc) == AKW_OK)

#endif // AKW_COMMON_H

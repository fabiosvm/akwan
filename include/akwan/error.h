//
// error.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_ERROR_H
#define AKW_ERROR_H

#define AKW_ERROR_MAX_LENGTH (511)

typedef char AkwError[AKW_ERROR_MAX_LENGTH + 1];

void akw_error_set(AkwError err, const char *fmt, ...);

#endif // AKW_ERROR_H

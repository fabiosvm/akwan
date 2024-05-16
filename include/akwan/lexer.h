//
// lexer.h
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AKW_LEXER_H
#define AKW_LEXER_H

#include "error.h"

typedef enum
{
  AKW_TOKEN_KIND_EOF,    AKW_TOKEN_KIND_COMMA,    AKW_TOKEN_KIND_SEMICOLON, AKW_TOKEN_KIND_LPAREN,
  AKW_TOKEN_KIND_RPAREN, AKW_TOKEN_KIND_LBRACKET, AKW_TOKEN_KIND_RBRACKET,  AKW_TOKEN_KIND_LBRACE,
  AKW_TOKEN_KIND_RBRACE, AKW_TOKEN_KIND_EQ,       AKW_TOKEN_KIND_PLUS,      AKW_TOKEN_KIND_MINUS,
  AKW_TOKEN_KIND_STAR,   AKW_TOKEN_KIND_SLASH,    AKW_TOKEN_KIND_PERCENT,   AKW_TOKEN_KIND_DOTDOT,
  AKW_TOKEN_KIND_INT,    AKW_TOKEN_KIND_NUMBER,   AKW_TOKEN_KIND_STRING,    AKW_TOKEN_KIND_FALSE_KW,
  AKW_TOKEN_KIND_LET_KW, AKW_TOKEN_KIND_NIL_KW,   AKW_TOKEN_KIND_RETURN_KW, AKW_TOKEN_KIND_TRUE_KW,
  AKW_TOKEN_KIND_NAME
} AkwTokenKind;

typedef struct
{
  AkwTokenKind kind;
  int          ln;
  int          col;
  int          length;
  char         *chars;
} AkwToken;

typedef struct
{
  char     *source;
  char     *curr;
  int      ln;
  int      col;
  AkwToken token;
} AkwLexer;

const char *akw_token_kind_name(AkwTokenKind kind);
void akw_lexer_init(AkwLexer *lex, char *source, int *rc, AkwError err);
void akw_lexer_next(AkwLexer *lex, int *rc, AkwError err);

#endif // AKW_LEXER_H

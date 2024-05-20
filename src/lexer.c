//
// lexer.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "akwan/common.h"

#define char_at(l, i)   ((l)->curr[(i)])
#define current_char(l) char_at(l, 0)

static inline void skip_space(AkwLexer *lex);
static inline void next_char(AkwLexer *lex);
static inline void next_chars(AkwLexer *lex, int length);
static inline bool match_char(AkwLexer *lex, char c, AkwTokenKind kind);
static inline bool match_chars(AkwLexer *lex, const char *chars, AkwTokenKind kind);
static inline bool match_keyword(AkwLexer *lex, const char *kw, AkwTokenKind kind);
static inline bool match_number(AkwLexer *lex);
static inline bool match_string(AkwLexer *lex, int *rc, AkwError err);
static inline bool match_name(AkwLexer *lex);
static inline AkwToken token(AkwLexer *lex, AkwTokenKind kind, int length,
  char *chars);

static inline void skip_space(AkwLexer *lex)
{
  while (isspace(current_char(lex)))
    next_char(lex);
}

static inline void next_char(AkwLexer *lex)
{
  if (current_char(lex) == '\n')
  {
    ++lex->ln;
    lex->col = 1;
    ++lex->curr;
    return;
  }
  ++lex->col;
  ++lex->curr;
}

static inline void next_chars(AkwLexer *lex, int length)
{
  for (int i = 0; i < length; ++i)
    next_char(lex);
}

static inline bool match_char(AkwLexer *lex, char c, AkwTokenKind kind)
{
  if (current_char(lex) != c)
    return false;
  lex->token = token(lex, kind, 1, lex->curr);
  next_char(lex);
  return true;
}

static inline bool match_chars(AkwLexer *lex, const char *chars, AkwTokenKind kind)
{
  int length = (int) strlen(chars);
  if (memcmp(lex->curr, chars, length))
    return false;
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_keyword(AkwLexer *lex, const char *kw, AkwTokenKind kind)
{
  int length = (int) strlen(kw);
  if (strncmp(lex->curr, kw, length)
   || (isalnum(char_at(lex, length)))
   || (char_at(lex, length) == '_'))
    return false;
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_number(AkwLexer *lex)
{
  int length = 0;
  if (char_at(lex, length) == '0')
    ++length;
  else
  {
    if (char_at(lex, length) < '1' || char_at(lex, length) > '9')
      return false;
    ++length;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  AkwTokenKind kind = AKW_TOKEN_KIND_INT;
  if (char_at(lex, length) == '.')
  {
    kind = AKW_TOKEN_KIND_NUMBER;
    if (!isdigit(char_at(lex, length + 1)))
      goto end;
    length += 2;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  if (char_at(lex, length) == 'e' || char_at(lex, length) == 'E')
  {
    ++length;
    if (char_at(lex, length) == '+' || char_at(lex, length) == '-')
      ++length;
    if (!isdigit(char_at(lex, length)))
      return false;
    ++length;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  if (isalnum(char_at(lex, length)) || char_at(lex, length) == '_')
    return false;
end:
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_string(AkwLexer *lex, int *rc, AkwError err)
{
  if (current_char(lex) != '\"') return false;
  int n = 1;
  for (;;)
  {
    if (char_at(lex, n) == '\"')
    {
      ++n;
      break;
    }
    if (char_at(lex, n) == '\0')
    {
      *rc = AKW_LEXICAL_ERROR;
      akw_error_set(err, "unterminated string in %d,%d", lex->ln, lex->col);
      return false;
    }
    ++n;
  }
  lex->token = token(lex, AKW_TOKEN_KIND_STRING, n - 2, &lex->curr[1]);
  next_chars(lex, n);
  return true;
}

static inline bool match_name(AkwLexer *lex)
{
  if (current_char(lex) != '_' && !isalpha(current_char(lex)))
    return false;
  int length = 1;
  while (char_at(lex, length) == '_' || isalnum(char_at(lex, length)))
    ++length;
  lex->token = token(lex, AKW_TOKEN_KIND_NAME, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline AkwToken token(AkwLexer *lex, AkwTokenKind kind, int length,
  char *chars)
{
  return (AkwToken) {
    .kind = kind,
    .ln = lex->ln,
    .col = lex->col,
    .length = length,
    .chars = chars
  };
}

const char *akw_token_kind_name(AkwTokenKind kind)
{
  char *name = "Eof";
  switch (kind)
  {
  case AKW_TOKEN_KIND_EOF:
    break;
  case AKW_TOKEN_KIND_COMMA:
    name = "Comma";
    break;
  case AKW_TOKEN_KIND_SEMICOLON:
    name = "Semicolon";
    break;
  case AKW_TOKEN_KIND_LPAREN:
    name = "LParen";
    break;
  case AKW_TOKEN_KIND_RPAREN:
    name = "RParen";
    break;
  case AKW_TOKEN_KIND_LBRACKET:
    name = "LBracket";
    break;
  case AKW_TOKEN_KIND_RBRACKET:
    name = "RBracket";
    break;
  case AKW_TOKEN_KIND_LBRACE:
    name = "LBrace";
    break;
  case AKW_TOKEN_KIND_RBRACE:
    name = "RBrace";
    break;
  case AKW_TOKEN_KIND_AMP:
    name = "Amp";
    break;
  case AKW_TOKEN_KIND_EQ:
    name = "Eq";
    break;
  case AKW_TOKEN_KIND_PLUS:
    name = "Plus";
    break;
  case AKW_TOKEN_KIND_MINUS:
    name = "Minus";
    break;
  case AKW_TOKEN_KIND_STAR:
    name = "Star";
    break;
  case AKW_TOKEN_KIND_SLASH:
    name = "Slash";
    break;
  case AKW_TOKEN_KIND_PERCENT:
    name = "Percent";
    break;
  case AKW_TOKEN_KIND_DOTDOT:
    name = "DotDot";
    break;
  case AKW_TOKEN_KIND_INT:
    name = "Int";
    break;
  case AKW_TOKEN_KIND_NUMBER:
    name = "Number";
    break;
  case AKW_TOKEN_KIND_STRING:
    name = "String";
    break;
  case AKW_TOKEN_KIND_FALSE_KW:
    name = "False";
    break;
  case AKW_TOKEN_KIND_LET_KW:
    name = "Let";
    break;
  case AKW_TOKEN_KIND_INOUT_KW:
    name = "Inout";
    break;
  case AKW_TOKEN_KIND_NIL_KW:
    name = "Nil";
    break;
  case AKW_TOKEN_KIND_RETURN_KW:
    name = "Return";
    break;
  case AKW_TOKEN_KIND_TRUE_KW:
    name = "True";
    break;
  case AKW_TOKEN_KIND_NAME:
    name = "Name";
    break;
  }
  return name;
}

void akw_lexer_init(AkwLexer *lex, char *source, int *rc, AkwError err)
{
  lex->source = source;
  lex->curr = source;
  lex->ln = 1;
  lex->col = 1;
  akw_lexer_next(lex, rc, err);
}

void akw_lexer_next(AkwLexer *lex, int *rc, AkwError err)
{
  skip_space(lex);
  if (match_char(lex, 0, AKW_TOKEN_KIND_EOF)) return;
  if (match_char(lex, ',', AKW_TOKEN_KIND_COMMA)) return;
  if (match_char(lex, ';', AKW_TOKEN_KIND_SEMICOLON)) return;
  if (match_char(lex, '(', AKW_TOKEN_KIND_LPAREN)) return;
  if (match_char(lex, ')', AKW_TOKEN_KIND_RPAREN)) return;
  if (match_char(lex, '[', AKW_TOKEN_KIND_LBRACKET)) return;
  if (match_char(lex, ']', AKW_TOKEN_KIND_RBRACKET)) return;
  if (match_char(lex, '{', AKW_TOKEN_KIND_LBRACE)) return;
  if (match_char(lex, '}', AKW_TOKEN_KIND_RBRACE)) return;
  if (match_char(lex, '&', AKW_TOKEN_KIND_AMP)) return;
  if (match_char(lex, '=', AKW_TOKEN_KIND_EQ)) return;
  if (match_char(lex, '+', AKW_TOKEN_KIND_PLUS)) return;
  if (match_char(lex, '-', AKW_TOKEN_KIND_MINUS)) return;
  if (match_char(lex, '*', AKW_TOKEN_KIND_STAR)) return;
  if (match_char(lex, '/', AKW_TOKEN_KIND_SLASH)) return;
  if (match_char(lex, '%', AKW_TOKEN_KIND_PERCENT)) return;
  if (match_chars(lex, "..", AKW_TOKEN_KIND_DOTDOT)) return;
  if (match_number(lex)) return;
  if (match_string(lex, rc, err) || !akw_is_ok(*rc)) return;
  if (match_keyword(lex, "false", AKW_TOKEN_KIND_FALSE_KW)) return;
  if (match_keyword(lex, "inout", AKW_TOKEN_KIND_INOUT_KW)) return;
  if (match_keyword(lex, "let", AKW_TOKEN_KIND_LET_KW)) return;
  if (match_keyword(lex, "nil", AKW_TOKEN_KIND_NIL_KW)) return;
  if (match_keyword(lex, "return", AKW_TOKEN_KIND_RETURN_KW)) return;
  if (match_keyword(lex, "true", AKW_TOKEN_KIND_TRUE_KW)) return;
  if (match_name(lex)) return;
  char c = current_char(lex);
  c = isprint(c) ? c : '?';
  *rc = AKW_LEXICAL_ERROR;
  akw_error_set(err, "unexpected character '%c' in %d,%d", c, lex->ln,
    lex->col);
}

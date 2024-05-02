//
// compiler.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "akwan/compiler.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "akwan/lexer.h"

#define match(c, t) ((c)->lex.token.kind == (t))

#define next(c) \
  do { \
    akw_lexer_next(&(c)->lex, &(c)->rc, (c)->err); \
    if (!akw_compiler_is_ok(c)) return; \
  } while (0)

#define consume(c, t) \
  do { \
    if (!match((c), (t))) { \
      unexpected_token_error(c); \
      return; \
    } \
    next(c); \
  } while (0)

#define emit(c, i) \
  do { \
    akw_chunk_emit(&(c)->chunk, (i), &(c)->rc); \
    if (akw_compiler_is_ok(c)) break; \
    assert((c)->rc == AKW_RANGE_ERROR); \
    akw_error_set((c)->err, "code too large"); \
    return; \
  } while (0)

static inline bool token_equal(AkwToken *token1, AkwToken *token2);
static inline int define_symbol(AkwCompiler *comp, AkwToken *name);
static inline int find_symbol(AkwCompiler *comp, AkwToken *name);
static inline void unexpected_token_error(AkwCompiler *comp);
static inline void compile_chunk(AkwCompiler *comp);
static inline void compile_stmt(AkwCompiler *comp);
static inline void compile_let_stmt(AkwCompiler *comp);
static inline void compile_return_stmt(AkwCompiler *comp);
static inline void compile_expr(AkwCompiler *comp, int dest);
static inline void compile_mul_expr(AkwCompiler *comp, int dest);
static inline void compile_unary_expr(AkwCompiler *comp, int dest);
static inline void compile_prim_expr(AkwCompiler *comp, int dest);

static inline bool token_equal(AkwToken *token1, AkwToken *token2)
{
  return token1->length == token2->length
    && !memcmp(token1->chars, token2->chars, token1->length);
}

static inline int define_symbol(AkwCompiler *comp, AkwToken *name)
{
  int n = comp->symbols.count;
  AkwSymbol *symbols = comp->symbols.elements;
  for (int i = n - 1; i > -1; --i)
  {
    AkwSymbol *symb = &symbols[i];
    if (token_equal(name, &symb->name))
    {
      comp->rc = AKW_SEMANTIC_ERROR;
      akw_error_set(comp->err, "symbol '%.*s' already defined", name->length,
        name->chars);
      return -1;
    }
  }
  if (n > UINT8_MAX)
  {
    comp->rc = AKW_SEMANTIC_ERROR;
    akw_error_set(comp->err, "too many symbols defined");
    return -1;
  }
  AkwSymbol symb = {
    .name = *name,
    .index = n
  };
  int rc = AKW_OK;
  akw_vector_append(&comp->symbols, symb, &rc);
  assert(rc == AKW_OK);
  return n;
}

static inline int find_symbol(AkwCompiler *comp, AkwToken *name)
{
  int n = comp->symbols.count;
  AkwSymbol *symbols = comp->symbols.elements;
  for (int i = n - 1; i > -1; --i)
  {
    AkwSymbol *symb = &symbols[i];
    if (token_equal(name, &symb->name))
      return symb->index;
  }
  comp->rc = AKW_SEMANTIC_ERROR;
  akw_error_set(comp->err, "symbol '%.*s' referenced but not defined",
    name->length, name->chars);
  return -1;
}

static inline void unexpected_token_error(AkwCompiler *comp)
{
  comp->rc = AKW_SYNTAX_ERROR;
  AkwToken *token = &comp->lex.token;
  if (token->kind == AKW_TOKEN_KIND_EOF)
  {
    akw_error_set(comp->err, "unexpected end of file");
    return;
  }
  akw_error_set(comp->err, "unexpected token '%.*s'", token->length,
    token->chars);
}

static inline void compile_chunk(AkwCompiler *comp)
{
  while (!match(comp, AKW_TOKEN_KIND_EOF))
  {
    compile_stmt(comp);
    if (!akw_compiler_is_ok(comp)) return;
  }
  emit(comp, akw_nil_instr(0));
  emit(comp, akw_return_instr());
}

static inline void compile_stmt(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_LET_KW))
  {
    next(comp);
    compile_let_stmt(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_RETURN_KW))
  {
    next(comp);
    compile_return_stmt(comp);
    return;
  }
  unexpected_token_error(comp);
}

static inline void compile_let_stmt(AkwCompiler *comp)
{
  if (!match(comp, AKW_TOKEN_KIND_NAME))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken token = comp->lex.token;
  next(comp);
  int dest = define_symbol(comp, &token);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_EQ);
  compile_expr(comp, dest);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
}

static inline void compile_return_stmt(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_SEMICOLON))
  {
    next(comp);
    return;
  }
  int dest = comp->symbols.count;
  compile_expr(comp, dest);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit(comp, akw_return_instr());
}

static inline void compile_expr(AkwCompiler *comp, int dest)
{
  compile_mul_expr(comp, dest);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_PLUS))
    {
      next(comp);
      compile_mul_expr(comp, dest + 1);
      if (!akw_compiler_is_ok(comp)) return;
      emit(comp, akw_add_instr(dest, dest, dest + 1));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_MINUS))
    {
      next(comp);
      compile_mul_expr(comp, dest + 1);
      if (!akw_compiler_is_ok(comp)) return;
      emit(comp, akw_sub_instr(dest, dest, dest + 1));
      continue;
    }
    break;
  }
}

static inline void compile_mul_expr(AkwCompiler *comp, int dest)
{
  compile_unary_expr(comp, dest);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_STAR))
    {
      next(comp);
      compile_unary_expr(comp, dest + 1);
      if (!akw_compiler_is_ok(comp)) return;
      emit(comp, akw_mul_instr(dest, dest, dest + 1));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_SLASH))
    {
      next(comp);
      compile_unary_expr(comp, dest + 1);
      if (!akw_compiler_is_ok(comp)) return;
      emit(comp, akw_div_instr(dest, dest, dest + 1));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_PERCENT))
    {
      next(comp);
      compile_unary_expr(comp, dest + 1);
      if (!akw_compiler_is_ok(comp)) return;
      emit(comp, akw_mod_instr(dest, dest, dest + 1));
      continue;
    }
    break;
  }
}

static inline void compile_unary_expr(AkwCompiler *comp, int dest)
{
  if (match(comp, AKW_TOKEN_KIND_MINUS))
  {
    next(comp);
    compile_unary_expr(comp, dest);
    if (!akw_compiler_is_ok(comp)) return;
    emit(comp, akw_neg_instr(dest, dest));
    return;
  }
  compile_prim_expr(comp, dest);
}

static inline void compile_prim_expr(AkwCompiler *comp, int dest)
{
  if (match(comp, AKW_TOKEN_KIND_NIL_KW))
  {
    next(comp);
    emit(comp, akw_nil_instr(dest));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_FALSE_KW))
  {
    next(comp);
    emit(comp, akw_false_instr(dest));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_TRUE_KW))
  {
    next(comp);
    emit(comp, akw_true_instr(dest));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_INT)
   || match(comp, AKW_TOKEN_KIND_NUMBER))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    double num = strtod(token.chars, NULL);
    AkwValue val = akw_number_value(num);
    int index = akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
    if (!akw_compiler_is_ok(comp)) return;
    emit(comp, akw_const_instr(dest, index));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NAME))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    int index = find_symbol(comp, &token);
    if (!akw_compiler_is_ok(comp)) return;
    emit(comp, akw_move_instr(dest, index));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LPAREN))
  {
    next(comp);
    compile_expr(comp, dest);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RPAREN);
    return;
  }
  unexpected_token_error(comp);
}

void akw_compiler_init(AkwCompiler *comp, char *file, char *source)
{
  comp->rc = AKW_OK;
  akw_lexer_init(&comp->lex, file, source, &comp->rc, comp->err);
  if (!akw_compiler_is_ok(comp)) return;
  akw_vector_init(&comp->symbols);
  akw_chunk_init(&comp->chunk);
}

void akw_compiler_deinit(AkwCompiler *comp)
{
  akw_vector_deinit(&comp->symbols);
  akw_chunk_deinit(&comp->chunk);
}

void akw_compiler_compile(AkwCompiler *comp)
{
  compile_chunk(comp);
}

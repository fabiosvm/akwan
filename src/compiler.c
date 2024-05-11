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
#include "akwan/string.h"

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

#define check_code(c) \
  do { \
    if (akw_compiler_is_ok(c)) break; \
    assert((c)->rc == AKW_RANGE_ERROR); \
    akw_error_set((c)->err, "code too large"); \
    return; \
  } while (0)

#define emit_opcode(c, op) \
  do { \
    akw_chunk_emit_opcode(&(c)->chunk, (op), &(c)->rc); \
    check_code(c); \
  } while (0)

#define emit_byte(c, b) \
  do { \
    akw_chunk_emit_byte(&(c)->chunk, (b), &(c)->rc); \
    check_code(c); \
  } while (0)

static inline bool token_equal(AkwToken *token1, AkwToken *token2);
static inline void define_symbol(AkwCompiler *comp, AkwToken *name);
static inline uint8_t find_symbol(AkwCompiler *comp, AkwToken *name);
static inline void unexpected_token_error(AkwCompiler *comp);
static inline void compile_chunk(AkwCompiler *comp);
static inline void compile_stmt(AkwCompiler *comp);
static inline void compile_let_stmt(AkwCompiler *comp);
static inline void compile_return_stmt(AkwCompiler *comp);
static inline void compile_expr(AkwCompiler *comp);
static inline void compile_add_expr(AkwCompiler *comp);
static inline void compile_mul_expr(AkwCompiler *comp);
static inline void compile_unary_expr(AkwCompiler *comp);
static inline void compile_prim_expr(AkwCompiler *comp);

static inline bool token_equal(AkwToken *token1, AkwToken *token2)
{
  return token1->length == token2->length
    && !memcmp(token1->chars, token2->chars, token1->length);
}

static inline void define_symbol(AkwCompiler *comp, AkwToken *name)
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
      return;
    }
  }
  if (n > UINT8_MAX)
  {
    comp->rc = AKW_SEMANTIC_ERROR;
    akw_error_set(comp->err, "too many symbols defined");
    return;
  }
  AkwSymbol symb = {
    .name = *name,
    .index = (uint8_t) n
  };
  int rc = AKW_OK;
  akw_vector_append(&comp->symbols, symb, &rc);
  assert(akw_is_ok(rc));
}

static inline uint8_t find_symbol(AkwCompiler *comp, AkwToken *name)
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
  return 0;
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
  emit_opcode(comp, AKW_OP_NIL);
  emit_opcode(comp, AKW_OP_RETURN);
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
  compile_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit_opcode(comp, AKW_OP_POP);
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
  consume(comp, AKW_TOKEN_KIND_EQ);
  compile_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  define_symbol(comp, &token);
}

static inline void compile_return_stmt(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_SEMICOLON))
  {
    next(comp);
    return;
  }
  compile_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit_opcode(comp, AKW_OP_RETURN);
}

static inline void compile_expr(AkwCompiler *comp)
{
  compile_add_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  if (match(comp, AKW_TOKEN_KIND_DOTDOT))
  {
    next(comp);
    compile_add_expr(comp);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_RANGE);
  }
}

static inline void compile_add_expr(AkwCompiler *comp)
{
  compile_mul_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_PLUS))
    {
      next(comp);
      compile_mul_expr(comp);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_ADD);
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_MINUS))
    {
      next(comp);
      compile_mul_expr(comp);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_SUB);
      continue;
    }
    break;
  }
}

static inline void compile_mul_expr(AkwCompiler *comp)
{
  compile_unary_expr(comp);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_STAR))
    {
      next(comp);
      compile_unary_expr(comp);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_MUL);
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_SLASH))
    {
      next(comp);
      compile_unary_expr(comp);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_DIV);
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_PERCENT))
    {
      next(comp);
      compile_unary_expr(comp);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_MOD);
      continue;
    }
    break;
  }
}

static inline void compile_unary_expr(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_MINUS))
  {
    next(comp);
    compile_unary_expr(comp);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_NEG);
    return;
  }
  compile_prim_expr(comp);
}

static inline void compile_prim_expr(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_NIL_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_NIL);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_FALSE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_FALSE);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_TRUE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_TRUE);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_INT)
   || match(comp, AKW_TOKEN_KIND_NUMBER))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    double num = strtod(token.chars, NULL);
    AkwValue val = akw_number_value(num);
    uint8_t index = (uint8_t) akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_CONST);
    emit_byte(comp, index);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_STRING))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    AkwString *str = akw_string_new_from(token.length, token.chars, &comp->rc);
    if (!akw_compiler_is_ok(comp)) return;
    AkwValue val = akw_string_value(str);
    uint8_t index = (uint8_t) akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_CONST);
    emit_byte(comp, index);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NAME))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    uint8_t index = find_symbol(comp, &token);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_LOAD);
    emit_byte(comp, index);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LPAREN))
  {
    next(comp);
    compile_expr(comp);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RPAREN);
    return;
  }
  unexpected_token_error(comp);
}

void akw_compiler_init(AkwCompiler *comp, char *source)
{
  comp->rc = AKW_OK;
  akw_lexer_init(&comp->lex, source, &comp->rc, comp->err);
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

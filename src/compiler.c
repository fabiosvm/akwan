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

#define set_type_info(d, s) \
  do { \
    if (!(d)) break; \
    *(d) = (s); \
  } while (0)

#define is_check_only(c) ((c)->flags & AKW_COMPILER_FLAG_CHECK_ONLY)

#define check_code(c) \
  do { \
    if (akw_compiler_is_ok(c)) break; \
    assert((c)->rc == AKW_RANGE_ERROR); \
    akw_error_set((c)->err, "code too large"); \
    return; \
  } while (0)

#define emit_opcode(c, op) \
  do { \
    if (is_check_only(c)) break; \
    akw_chunk_emit_opcode(&(c)->chunk, (op), &(c)->rc); \
    check_code(c); \
  } while (0)

#define emit_byte(c, b) \
  do { \
    if (is_check_only(c)) break; \
    akw_chunk_emit_byte(&(c)->chunk, (b), &(c)->rc); \
    check_code(c); \
  } while (0)

#define push_scope(c) \
  do { \
    ++(c)->scopeDepth; \
  } while (0)

static inline bool token_equal(AkwToken *token1, AkwToken *token2);
static inline void define_variable(AkwCompiler *comp, AkwToken *name,
  AkwTypeInfo info);
static inline AkwVariable *find_variable(AkwCompiler *comp, AkwToken *name);
static inline void pop_scope(AkwCompiler *comp);
static inline void unexpected_token_error(AkwCompiler *comp);
static inline void compile_chunk(AkwCompiler *comp);
static inline void compile_stmt(AkwCompiler *comp);
static inline void compile_let_stmt(AkwCompiler *comp);
static inline void compile_inout_stmt(AkwCompiler *comp);
static inline void compile_assign_stmt(AkwCompiler *comp);
static inline void compile_return_stmt(AkwCompiler *comp);
static inline void compile_block_stmt(AkwCompiler *comp);
static inline void compile_expr(AkwCompiler *comp, AkwTypeInfo *info);
static inline void compile_add_expr(AkwCompiler *comp, AkwTypeInfo *info);
static inline void compile_mul_expr(AkwCompiler *comp, AkwTypeInfo *info);
static inline void compile_unary_expr(AkwCompiler *comp, AkwTypeInfo *info);
static inline void compile_prim_expr(AkwCompiler *comp, AkwTypeInfo *info);
static inline void compile_int(AkwCompiler *comp);
static inline void compile_number(AkwCompiler *comp);
static inline void compile_string(AkwCompiler *comp);
static inline void compile_array(AkwCompiler *comp);
static inline void compile_ref(AkwCompiler *comp);
static inline void compile_vasiable(AkwCompiler *comp, AkwTypeInfo *info);

static inline bool token_equal(AkwToken *token1, AkwToken *token2)
{
  return token1->length == token2->length
    && !memcmp(token1->chars, token2->chars, token1->length);
}

static inline void define_variable(AkwCompiler *comp, AkwToken *name,
  AkwTypeInfo info)
{
  int n = comp->variables.count;
  AkwVariable *variables = comp->variables.elements;
  for (int i = n - 1; i > -1; --i)
  {
    AkwVariable *var = &variables[i];
    if (var->depth < comp->scopeDepth) break;
    if (token_equal(name, &var->name))
    {
      comp->rc = AKW_SEMANTIC_ERROR;
      akw_error_set(comp->err, "variable '%.*s' already defined in %d,%d",
        name->length, name->chars, name->ln, name->col);
      return;
    }
  }
  if (n > UINT8_MAX)
  {
    comp->rc = AKW_SEMANTIC_ERROR;
    akw_error_set(comp->err, "too many variables defined in %d,%d",
      name->ln, name->col);
    return;
  }
  AkwVariable var = {
    .name = *name,
    .depth = comp->scopeDepth,
    .info = info,
    .index = (uint8_t) n
  };
  int rc = AKW_OK;
  akw_vector_append(&comp->variables, var, &rc);
  assert(akw_is_ok(rc));
}

static inline AkwVariable *find_variable(AkwCompiler *comp, AkwToken *name)
{
  int n = comp->variables.count;
  AkwVariable *variables = comp->variables.elements;
  int scopeDepth = comp->scopeDepth;
  for (int i = n - 1; i > -1; --i)
  {
    AkwVariable *var = &variables[i];
    if (var->depth > scopeDepth) continue;
    if (var->depth < scopeDepth) break;
    if (token_equal(name, &var->name))
      return var;
  }
  comp->rc = AKW_SEMANTIC_ERROR;
  akw_error_set(comp->err, "variable '%.*s' used but not defined in %d,%d",
    name->length, name->chars, name->ln, name->col);
  return NULL;
}

static inline void pop_scope(AkwCompiler *comp)
{
  int n = comp->variables.count;
  AkwVariable *variables = comp->variables.elements;
  int scopeDepth = comp->scopeDepth;
  for (int i = n - 1; i > -1; --i)
  {
    AkwVariable *var = &variables[i];
    if (var->depth > scopeDepth) continue;
    if (var->depth < scopeDepth) break;
    emit_opcode(comp, AKW_OP_POP);
  }
  --comp->scopeDepth;
}

static inline void unexpected_token_error(AkwCompiler *comp)
{
  comp->rc = AKW_SYNTAX_ERROR;
  AkwToken *token = &comp->lex.token;
  if (token->kind == AKW_TOKEN_KIND_EOF)
  {
    akw_error_set(comp->err, "unexpected end of file in %d,%d",
      token->ln, token->col);
    return;
  }
  akw_error_set(comp->err, "unexpected token '%.*s' in %d,%d",
    token->length, token->chars, token->ln, token->col);
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
    compile_let_stmt(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_INOUT_KW))
  {
    compile_inout_stmt(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NAME))
  {
    compile_assign_stmt(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_RETURN_KW))
  {
    compile_return_stmt(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LBRACE))
  {
    compile_block_stmt(comp);
    return;
  }
  compile_expr(comp, NULL);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit_opcode(comp, AKW_OP_POP);
}

static inline void compile_let_stmt(AkwCompiler *comp)
{
  next(comp);
  if (!match(comp, AKW_TOKEN_KIND_NAME))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken token = comp->lex.token;
  next(comp);
  if (match(comp, AKW_TOKEN_KIND_EQ))
  {
    next(comp);
    compile_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
  }
  else
    emit_opcode(comp, AKW_OP_NIL);
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  define_variable(comp, &token, akw_type_info(false));
}

static inline void compile_inout_stmt(AkwCompiler *comp)
{
  next(comp);
  if (!match(comp, AKW_TOKEN_KIND_NAME))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken token = comp->lex.token;
  next(comp);
  consume(comp, AKW_TOKEN_KIND_EQ);
  AkwTypeInfo rhsInfo;
  compile_expr(comp, &rhsInfo);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  define_variable(comp, &token, akw_type_info(true));
  if (!akw_compiler_is_ok(comp)) return;
  if (rhsInfo.isRef) return;
  comp->rc = AKW_TYPE_ERROR;
  akw_error_set(comp->err, "cannot pass a value to the inout variable '%.*s' in %d,%d",
    token.length, token.chars, token.ln, token.col);
}

static inline void compile_assign_stmt(AkwCompiler *comp)
{
  AkwToken token = comp->lex.token;
  next(comp);
  consume(comp, AKW_TOKEN_KIND_EQ);
  AkwTypeInfo rhsInfo;
  compile_expr(comp, &rhsInfo);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  AkwVariable *var = find_variable(comp, &token);
  if (!akw_compiler_is_ok(comp)) return;
  AkwOpcode op = var->info.isRef ? AKW_OP_SET_LOCAL_BY_REF : AKW_OP_SET_LOCAL;
  emit_opcode(comp, op);
  emit_byte(comp, var->index); 
}

static inline void compile_return_stmt(AkwCompiler *comp)
{
  next(comp);
  if (match(comp, AKW_TOKEN_KIND_SEMICOLON))
  {
    next(comp);
    return;
  }
  compile_expr(comp, NULL);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit_opcode(comp, AKW_OP_RETURN);
}

static inline void compile_block_stmt(AkwCompiler *comp)
{
  next(comp);
  push_scope(comp);
  while (!match(comp, AKW_TOKEN_KIND_RBRACE))
  {
    compile_stmt(comp);
    if (!akw_compiler_is_ok(comp)) return;
  }
  next(comp);
  pop_scope(comp);
}

static inline void compile_expr(AkwCompiler *comp, AkwTypeInfo *info)
{
  compile_add_expr(comp, info);
  if (!akw_compiler_is_ok(comp)) return;
  if (match(comp, AKW_TOKEN_KIND_DOTDOT))
  {
    next(comp);
    compile_add_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_RANGE);
    set_type_info(info, akw_type_info(false));
  }
}

static inline void compile_add_expr(AkwCompiler *comp, AkwTypeInfo *info)
{
  compile_mul_expr(comp, info);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_PLUS))
    {
      next(comp);
      compile_mul_expr(comp, NULL);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_ADD);
      set_type_info(info, akw_type_info(false));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_MINUS))
    {
      next(comp);
      compile_mul_expr(comp, NULL);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_SUB);
      set_type_info(info, akw_type_info(false));
      continue;
    }
    break;
  }
}

static inline void compile_mul_expr(AkwCompiler *comp, AkwTypeInfo *info)
{
  compile_unary_expr(comp, info);
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_STAR))
    {
      next(comp);
      compile_unary_expr(comp, NULL);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_MUL);
      set_type_info(info, akw_type_info(false));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_SLASH))
    {
      next(comp);
      compile_unary_expr(comp, NULL);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_DIV);
      set_type_info(info, akw_type_info(false));
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_PERCENT))
    {
      next(comp);
      compile_unary_expr(comp, NULL);
      if (!akw_compiler_is_ok(comp)) return;
      emit_opcode(comp, AKW_OP_MOD);
      set_type_info(info, akw_type_info(false));
      continue;
    }
    break;
  }
}

static inline void compile_unary_expr(AkwCompiler *comp, AkwTypeInfo *info)
{
  if (match(comp, AKW_TOKEN_KIND_MINUS))
  {
    next(comp);
    compile_unary_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
    emit_opcode(comp, AKW_OP_NEG);
    set_type_info(info, akw_type_info(false));
    return;
  }
  compile_prim_expr(comp, info);
}

static inline void compile_prim_expr(AkwCompiler *comp, AkwTypeInfo *info)
{
  if (match(comp, AKW_TOKEN_KIND_NIL_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_NIL);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_FALSE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_FALSE);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_TRUE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_TRUE);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_INT))
  {
    compile_int(comp);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NUMBER))
  {
    compile_number(comp);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_STRING))
  {
    compile_string(comp);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LBRACKET))
  {
    compile_array(comp);
    set_type_info(info, akw_type_info(false));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_AMP))
  {
    compile_ref(comp);
    set_type_info(info, akw_type_info(true));
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NAME))
  {
    compile_vasiable(comp, info);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LPAREN))
  {
    next(comp);
    compile_expr(comp, info);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RPAREN);
    return;
  }
  unexpected_token_error(comp);
}

static inline void compile_int(AkwCompiler *comp)
{
  AkwToken token = comp->lex.token;
  next(comp);
  if (is_check_only(comp)) return;
  int64_t num = strtoll(token.chars, NULL, 10);
  if (num <= UINT8_MAX)
  {
    emit_opcode(comp, AKW_OP_INT);
    emit_byte(comp, (uint8_t) num);
    return;
  }
  AkwValue val = akw_int_value(num);
  uint8_t index = (uint8_t) akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
  if (!akw_compiler_is_ok(comp)) return;
  emit_opcode(comp, AKW_OP_CONST);
  emit_byte(comp, index);
}

static inline void compile_number(AkwCompiler *comp)
{
  AkwToken token = comp->lex.token;
  next(comp);
  if (is_check_only(comp)) return;
  double num = strtod(token.chars, NULL);
  AkwValue val = akw_number_value(num);
  uint8_t index = (uint8_t) akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
  if (!akw_compiler_is_ok(comp)) return;
  emit_opcode(comp, AKW_OP_CONST);
  emit_byte(comp, index);
}

static inline void compile_string(AkwCompiler *comp)
{
  AkwToken token = comp->lex.token;
  next(comp);
  if (is_check_only(comp)) return;
  AkwString *str = akw_string_new_from(token.length, token.chars, &comp->rc);
  if (!akw_compiler_is_ok(comp)) return;
  AkwValue val = akw_string_value(str);
  uint8_t index = (uint8_t) akw_chunk_append_constant(&comp->chunk, val, &comp->rc);
  if (!akw_compiler_is_ok(comp)) return;
  emit_opcode(comp, AKW_OP_CONST);
  emit_byte(comp, index);
}

static inline void compile_array(AkwCompiler *comp)
{
  next(comp);
  if (match(comp, AKW_TOKEN_KIND_RBRACKET))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_ARRAY);
    emit_byte(comp, 0);
    return;
  }
  compile_expr(comp, NULL);
  if (!akw_compiler_is_ok(comp)) return;
  uint8_t n = 1;
  while (match(comp, AKW_TOKEN_KIND_COMMA))
  {
    next(comp);
    compile_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
    ++n;
  }
  consume(comp, AKW_TOKEN_KIND_RBRACKET);
  emit_opcode(comp, AKW_OP_ARRAY);
  emit_byte(comp, n);
}

static inline void compile_ref(AkwCompiler *comp)
{
  next(comp);
  if (!match(comp, AKW_TOKEN_KIND_NAME))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken token = comp->lex.token;
  next(comp);
  AkwVariable *var = find_variable(comp, &token);
  if (!akw_compiler_is_ok(comp)) return;
  if (!match(comp, AKW_TOKEN_KIND_LBRACKET))
  {
    emit_opcode(comp, AKW_OP_LOCAL_REF);
    emit_byte(comp, var->index);
    return;
  }
  emit_opcode(comp, AKW_OP_GET_LOCAL);
  emit_byte(comp, var->index);
  for (;;)
  {
    next(comp);
    compile_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RBRACKET);
    if (match(comp, AKW_TOKEN_KIND_LBRACKET))
    {
      emit_opcode(comp, AKW_OP_GET_ELEMENT);
      continue;
    }
    break;
  }
  emit_opcode(comp, AKW_OP_ELEMENT_REF);
}

static inline void compile_vasiable(AkwCompiler *comp, AkwTypeInfo *info)
{
  AkwToken token = comp->lex.token;
  next(comp);
  AkwVariable *var = find_variable(comp, &token);
  if (!akw_compiler_is_ok(comp)) return;
  AkwOpcode op = var->info.isRef ? AKW_OP_GET_LOCAL_BY_REF : AKW_OP_GET_LOCAL;
  emit_opcode(comp, op);
  emit_byte(comp, var->index);
  while (match(comp, AKW_TOKEN_KIND_LBRACKET))
  {
    next(comp);
    compile_expr(comp, NULL);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RBRACKET);
    emit_opcode(comp, AKW_OP_GET_ELEMENT);
  }
  set_type_info(info, var->info);
}

void akw_compiler_init(AkwCompiler *comp, int flags, char *source)
{
  comp->flags = flags;
  comp->rc = AKW_OK;
  akw_lexer_init(&comp->lex, source, &comp->rc, comp->err);
  if (!akw_compiler_is_ok(comp)) return;
  comp->scopeDepth = 0;
  akw_vector_init(&comp->variables);
  akw_chunk_init(&comp->chunk);
}

void akw_compiler_deinit(AkwCompiler *comp)
{
  akw_vector_deinit(&comp->variables);
  akw_chunk_deinit(&comp->chunk);
}

void akw_compiler_compile(AkwCompiler *comp)
{
  compile_chunk(comp);
}

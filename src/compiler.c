//
// compiler.c
// 
// Copyright 2024 Fábio de Souza Villaça Medeiros
// 
// This file is part of the Akwan Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

/*
chunk         ::= stmt* EOF

stmt          ::= "let" NAME ( ":" type )? ( "=" expr )? ";"
                | NAME "=" expr ";"
                | "return" expr? ";"
                | "{" stmt* "}"
                | expr ";"

type          ::= "Bool" | "Number"

expr          ::= add_expr ( ".." add_expr )?

add_expr      ::= mul_expr ( ( "+" | "-" ) mul_expr )*

mul_expr      ::= unary_expr ( ( "*" | "/" | "%" ) unary_expr )*

unary_expr    ::= "-" unary_expr | prim_expr

prim_expr     ::= "nil" | "false" | "true" | INT | NUMBER | STRING
                | "[" ( expr ( "," expr )* )? "]"
                | NAME ( "[" expr "]" )*
                | "(" expr ")"
*/

/*
  Bool   = Bool
  Number = Number
  Number .. Number -> Range
  Number + Number -> Number
  Number - Number -> Number
  Number * Number -> Number
  Number / Number -> Number
  Number % Number -> Number
  - Number -> Number
*/

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
static inline void define_symbol(AkwCompiler *comp, AkwToken *name, AkwDataType dataType);
static inline AkwSymbol *find_symbol(AkwCompiler *comp, AkwToken *name);
static inline void pop_scope(AkwCompiler *comp);
static inline void unexpected_token_error(AkwCompiler *comp);
static inline void compile_chunk(AkwCompiler *comp);
static inline void compile_stmt(AkwCompiler *comp);
static inline void compile_let_stmt(AkwCompiler *comp);
static inline void compile_type(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_assign_stmt(AkwCompiler *comp);
static inline void compile_return_stmt(AkwCompiler *comp);
static inline void compile_block_stmt(AkwCompiler *comp);
static inline void compile_expr(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_add_expr(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_mul_expr(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_unary_expr(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_prim_expr(AkwCompiler *comp, AkwDataType *dataType);
static inline void compile_int(AkwCompiler *comp);
static inline void compile_number(AkwCompiler *comp);
static inline void compile_string(AkwCompiler *comp);
static inline void compile_array(AkwCompiler *comp);
static inline void compile_symbol(AkwCompiler *comp, AkwDataType *dataType);
static inline void check_assign_op(AkwCompiler *comp, AkwDataType lhsDataType, AkwDataType rhsDataType,
  AkwToken *token);
static inline void emit_binary_op(AkwCompiler *comp, AkwDataType lhsDataType, AkwDataType rhsDataType,
  AkwToken *token, AkwOpcode op);
static inline void emit_unary_op(AkwCompiler *comp, AkwDataType dataType, AkwToken *token, AkwOpcode op);

static inline bool token_equal(AkwToken *token1, AkwToken *token2)
{
  return token1->length == token2->length
    && !memcmp(token1->chars, token2->chars, token1->length);
}

static inline void define_symbol(AkwCompiler *comp, AkwToken *name, AkwDataType dataType)
{
  int n = comp->symbols.count;
  AkwSymbol *symbols = comp->symbols.elements;
  for (int i = n - 1; i > -1; --i)
  {
    AkwSymbol *symb = &symbols[i];
    if (symb->depth < comp->scopeDepth) break;
    if (token_equal(name, &symb->name))
    {
      comp->rc = AKW_SEMANTIC_ERROR;
      akw_error_set(comp->err, "symbol '%.*s' already defined in %d,%d",
        name->length, name->chars, name->ln, name->col);
      return;
    }
  }
  if (n > UINT8_MAX)
  {
    comp->rc = AKW_SEMANTIC_ERROR;
    akw_error_set(comp->err, "too many symbols defined in %d,%d",
      name->ln, name->col);
    return;
  }
  AkwSymbol symb = {
    .name = *name,
    .depth = comp->scopeDepth,
    .dataType = dataType,
    .index = (uint8_t) n
  };
  int rc = AKW_OK;
  akw_vector_append(&comp->symbols, symb, &rc);
  assert(akw_is_ok(rc));
}

static inline AkwSymbol *find_symbol(AkwCompiler *comp, AkwToken *name)
{
  int n = comp->symbols.count;
  AkwSymbol *symbols = comp->symbols.elements;
  int scopeDepth = comp->scopeDepth;
  for (int i = n - 1; i > -1; --i)
  {
    AkwSymbol *symb = &symbols[i];
    if (symb->depth > scopeDepth) continue;
    if (symb->depth < scopeDepth) break;
    if (token_equal(name, &symb->name))
      return symb;
  }
  comp->rc = AKW_SEMANTIC_ERROR;
  akw_error_set(comp->err, "symbol '%.*s' referenced but not defined in %d,%d",
    name->length, name->chars, name->ln, name->col);
  return NULL;
}

static inline void pop_scope(AkwCompiler *comp)
{
  int n = comp->symbols.count;
  AkwSymbol *symbols = comp->symbols.elements;
  int scopeDepth = comp->scopeDepth;
  for (int i = n - 1; i > -1; --i)
  {
    AkwSymbol *symb = &symbols[i];
    if (symb->depth > scopeDepth) continue;
    if (symb->depth < scopeDepth) break;
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

/*
stmt          ::= "let" NAME ( ":" type )? ( "=" expr )? ";"
                | NAME "=" expr ";"
                | "return" expr? ";"
                | "{" stmt* "}"
                | expr ";"

type          ::= "Bool" | "Number"
*/
static inline void compile_stmt(AkwCompiler *comp)
{
  if (match(comp, AKW_TOKEN_KIND_LET_KW))
  {
    compile_let_stmt(comp);
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
  AkwDataType dataType;
  compile_expr(comp, &dataType);
  if (!akw_compiler_is_ok(comp)) return;
  (void) dataType;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  emit_opcode(comp, AKW_OP_POP);
}

/*
stmt          ::= "let" NAME ( ":" type )? ( "=" expr )? ";"
type          ::= "Bool" | "Number"
*/
static inline void compile_let_stmt(AkwCompiler *comp)
{
  next(comp);
  if (!match(comp, AKW_TOKEN_KIND_NAME))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken name = comp->lex.token;
  next(comp);
  AkwDataType dataType = AKW_DATA_TYPE_ANY;
  if (match(comp, AKW_TOKEN_KIND_COLON))
  {
    compile_type(comp, &dataType);
    if (!akw_compiler_is_ok(comp)) return;
  }
  if (match(comp, AKW_TOKEN_KIND_EQ))
  {
    AkwToken opToken = comp->lex.token;
    next(comp);
    AkwDataType exprDataType;
    compile_expr(comp, &exprDataType);
    if (!akw_compiler_is_ok(comp)) return;
    check_assign_op(comp, dataType, exprDataType, &opToken);
    if (!akw_compiler_is_ok(comp)) return;
  }
  else
  {
    // TODO: Emit aproppriate opcode for the data type.
    emit_opcode(comp, AKW_OP_NIL);
  }
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  define_symbol(comp, &name, dataType);
}

static inline void compile_type(AkwCompiler *comp, AkwDataType *dataType)
{
  next(comp);
  if (match(comp, AKW_TOKEN_KIND_BOOL_KW))
  {
    next(comp);
    *dataType = AKW_DATA_TYPE_BOOL;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NUMBER_KW))
  {
    next(comp);
    *dataType = AKW_DATA_TYPE_NUMBER;
    return;
  }
  AkwToken *token = &comp->lex.token;
  comp->rc = AKW_SYNTAX_ERROR;
  akw_error_set(comp->err, "expected type but got '%.*s' in %d,%d",
    token->length, token->chars, token->ln, token->col);
}

/*
  Bool   = Bool
  Number = Number
*/
static inline void compile_assign_stmt(AkwCompiler *comp)
{
  AkwToken name = comp->lex.token;
  next(comp);
  if (!match(comp, AKW_TOKEN_KIND_EQ))
  {
    unexpected_token_error(comp);
    return;
  }
  AkwToken opToken = comp->lex.token;
  next(comp);
  AkwDataType exprDataType;
  compile_expr(comp, &exprDataType);
  if (!akw_compiler_is_ok(comp)) return;
  consume(comp, AKW_TOKEN_KIND_SEMICOLON);
  AkwSymbol *symb = find_symbol(comp, &name);
  if (!akw_compiler_is_ok(comp)) return;
  check_assign_op(comp, symb->dataType, exprDataType, &opToken);
  if (!akw_compiler_is_ok(comp)) return;
  emit_opcode(comp, AKW_OP_SET_LOCAL);
  emit_byte(comp, symb->index);
}

static inline void compile_return_stmt(AkwCompiler *comp)
{
  next(comp);
  if (match(comp, AKW_TOKEN_KIND_SEMICOLON))
  {
    next(comp);
    return;
  }
  AkwDataType dataType;
  compile_expr(comp, &dataType);
  if (!akw_compiler_is_ok(comp)) return;
  (void) dataType;
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

/*
  Number .. Number -> Range
*/
static inline void compile_expr(AkwCompiler *comp, AkwDataType *dataType)
{
  AkwDataType lhsDataType;
  compile_add_expr(comp, &lhsDataType);
  if (!akw_compiler_is_ok(comp)) return;
  *dataType = lhsDataType;
  if (match(comp, AKW_TOKEN_KIND_DOTDOT))
  {
    AkwToken token = comp->lex.token;
    next(comp);
    AkwDataType rhsDataType;
    compile_add_expr(comp, &rhsDataType);
    if (!akw_compiler_is_ok(comp)) return;
    emit_binary_op(comp, lhsDataType, rhsDataType, &token, AKW_OP_RANGE);
    if (!akw_compiler_is_ok(comp)) return;
    *dataType = AKW_DATA_TYPE_RANGE;
  }
}

/*
  Number + Number -> Number
  Number - Number -> Number
*/
static inline void compile_add_expr(AkwCompiler *comp, AkwDataType *dataType)
{
  AkwDataType lhsDataType;
  compile_mul_expr(comp, &lhsDataType);
  if (!akw_compiler_is_ok(comp)) return;
  *dataType = lhsDataType;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_PLUS))
    {
      AkwToken token = comp->lex.token;
      next(comp);
      AkwDataType rhsDataType;
      compile_mul_expr(comp, &rhsDataType);
      if (!akw_compiler_is_ok(comp)) return;
      emit_binary_op(comp, lhsDataType, rhsDataType, &token, AKW_OP_ADD);
      if (!akw_compiler_is_ok(comp)) return;
      *dataType = AKW_DATA_TYPE_NUMBER;
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_MINUS))
    {
      AkwToken token = comp->lex.token;
      next(comp);
      AkwDataType rhsDataType;
      compile_mul_expr(comp, &rhsDataType);
      if (!akw_compiler_is_ok(comp)) return;
      emit_binary_op(comp, lhsDataType, rhsDataType, &token, AKW_OP_SUB);
      if (!akw_compiler_is_ok(comp)) return;
      *dataType = AKW_DATA_TYPE_NUMBER;
      continue;
    }
    break;
  }
}

/*
  Number * Number -> Number
  Number / Number -> Number
  Number % Number -> Number
*/
static inline void compile_mul_expr(AkwCompiler *comp, AkwDataType *dataType)
{
  AkwDataType lhsDataType;  
  compile_unary_expr(comp, &lhsDataType);
  *dataType = lhsDataType;
  if (!akw_compiler_is_ok(comp)) return;
  for (;;)
  {
    if (match(comp, AKW_TOKEN_KIND_STAR))
    {
      AkwToken token = comp->lex.token;
      next(comp);
      AkwDataType rhsDataType;
      compile_unary_expr(comp, &rhsDataType);
      if (!akw_compiler_is_ok(comp)) return;
      emit_binary_op(comp, lhsDataType, rhsDataType, &token, AKW_OP_MUL);
      if (!akw_compiler_is_ok(comp)) return;
      *dataType = AKW_DATA_TYPE_NUMBER;
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_SLASH))
    {
      AkwToken token = comp->lex.token;
      next(comp);
      AkwDataType rhsDataType;
      compile_unary_expr(comp, &rhsDataType);
      if (!akw_compiler_is_ok(comp)) return;
      emit_binary_op(comp, lhsDataType, rhsDataType, &token, AKW_OP_DIV);
      if (!akw_compiler_is_ok(comp)) return;
      *dataType = AKW_DATA_TYPE_NUMBER;
      continue;
    }
    if (match(comp, AKW_TOKEN_KIND_PERCENT))
    {
      next(comp);
      AkwDataType rhsDataType;
      compile_unary_expr(comp, &rhsDataType);
      if (!akw_compiler_is_ok(comp)) return;
      emit_binary_op(comp, lhsDataType, rhsDataType, &comp->lex.token, AKW_OP_MOD);
      if (!akw_compiler_is_ok(comp)) return;
      *dataType = AKW_DATA_TYPE_NUMBER;
      continue;
    }
    break;
  }
}

/*
  - Number -> Number
*/
static inline void compile_unary_expr(AkwCompiler *comp, AkwDataType *dataType)
{
  if (match(comp, AKW_TOKEN_KIND_MINUS))
  {
    next(comp);
    AkwDataType exprDataType;
    compile_unary_expr(comp, &exprDataType);
    if (!akw_compiler_is_ok(comp)) return;
    emit_unary_op(comp, exprDataType, &comp->lex.token, AKW_OP_NEG);
    if (!akw_compiler_is_ok(comp)) return;
    *dataType = AKW_DATA_TYPE_NUMBER;
    return;
  }
  compile_prim_expr(comp, dataType);
}

static inline void compile_prim_expr(AkwCompiler *comp, AkwDataType *dataType)
{
  if (match(comp, AKW_TOKEN_KIND_NIL_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_NIL);
    *dataType = AKW_DATA_TYPE_NIL;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_FALSE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_FALSE);
    *dataType = AKW_DATA_TYPE_BOOL;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_TRUE_KW))
  {
    next(comp);
    emit_opcode(comp, AKW_OP_TRUE);
    *dataType = AKW_DATA_TYPE_BOOL;
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
    *dataType = AKW_DATA_TYPE_NUMBER;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_STRING))
  {
    compile_string(comp);
    *dataType = AKW_DATA_TYPE_STRING;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LBRACKET))
  {
    compile_array(comp);
    *dataType = AKW_DATA_TYPE_ARRAY;
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_NAME))
  {
    compile_symbol(comp, dataType);
    return;
  }
  if (match(comp, AKW_TOKEN_KIND_LPAREN))
  {
    next(comp);
    compile_expr(comp, dataType);
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
  AkwDataType dataType;
  compile_expr(comp, &dataType);
  if (!akw_compiler_is_ok(comp)) return;
  uint8_t n = 1;
  while (match(comp, AKW_TOKEN_KIND_COMMA))
  {
    next(comp);
    compile_expr(comp, &dataType);
    if (!akw_compiler_is_ok(comp)) return;
    ++n;
  }
  consume(comp, AKW_TOKEN_KIND_RBRACKET);
  emit_opcode(comp, AKW_OP_ARRAY);
  emit_byte(comp, n);
}

static inline void compile_symbol(AkwCompiler *comp, AkwDataType *dataType)
{
  AkwToken token = comp->lex.token;
  next(comp);
  AkwSymbol *symb = find_symbol(comp, &token);
  if (!akw_compiler_is_ok(comp)) return;
  emit_opcode(comp, AKW_OP_GET_LOCAL);
  emit_byte(comp, symb->index);
  if (!match(comp, AKW_TOKEN_KIND_LBRACKET))
  {
    *dataType = symb->dataType;
    return;
  }
  do
  {
    next(comp);
    compile_expr(comp, dataType);
    if (!akw_compiler_is_ok(comp)) return;
    consume(comp, AKW_TOKEN_KIND_RBRACKET);
    emit_opcode(comp, AKW_OP_GET_ELEMENT);
  }
  while (match(comp, AKW_TOKEN_KIND_LBRACKET));
}

static inline void check_assign_op(AkwCompiler *comp, AkwDataType lhsDataType, AkwDataType rhsDataType,
  AkwToken *token)
{
  if (lhsDataType == AKW_DATA_TYPE_ANY) return;
  if (lhsDataType == rhsDataType) return;
  comp->rc = AKW_TYPE_ERROR;
  akw_error_set(comp->err, "expected %s type but got %s in %d,%d", akw_data_type_name(lhsDataType),
    akw_data_type_name(rhsDataType), token->ln, token->col);
}

static inline void emit_binary_op(AkwCompiler *comp, AkwDataType lhsDataType, AkwDataType rhsDataType,
  AkwToken *token, AkwOpcode op)
{
  switch (op)
  {
  case AKW_OP_RANGE:
    {
      // TODO: Check data type.
      (void) token;
    }
    break;
  case AKW_OP_ADD:
    {
      // TODO: Check data type.
      if (lhsDataType == AKW_DATA_TYPE_NUMBER && rhsDataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_ADD;
    }
    break;
  case AKW_OP_SUB:
    {
      // TODO: Check data type.
      if (lhsDataType == AKW_DATA_TYPE_NUMBER && rhsDataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_SUB;
    }
    break;
  case AKW_OP_MUL:
    {
      // TODO: Check data type.
      if (lhsDataType == AKW_DATA_TYPE_NUMBER && rhsDataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_MUL;
    }
    break;
  case AKW_OP_DIV:
    {
      // TODO: Check data type.
      if (lhsDataType == AKW_DATA_TYPE_NUMBER && rhsDataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_DIV;
    }
    break;
  case AKW_OP_MOD:
    {
      // TODO: Check data type.
      if (lhsDataType == AKW_DATA_TYPE_NUMBER && rhsDataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_MOD;
    }
    break;
  default:
    assert(0);
  }
  emit_opcode(comp, op);
}

static inline void emit_unary_op(AkwCompiler *comp, AkwDataType dataType, AkwToken *token, AkwOpcode op)
{
  switch (op)
  {
  case AKW_OP_NEG:
    {
      // TODO: Check data type.
      (void) token;
      if (dataType == AKW_DATA_TYPE_NUMBER)
        op = AKW_OP_NUMBER_NEG;
    }
    break;
  default:
    assert(0);
  }
  emit_opcode(comp, op);
}

const char *akw_data_type_name(AkwDataType dataType)
{
  char *name = "Any";
  switch (dataType)
  {
  case AKW_DATA_TYPE_ANY:
    break;
  case AKW_DATA_TYPE_NIL:
    name = "Nil";
    break;
  case AKW_DATA_TYPE_BOOL:
    name = "Bool";
    break;
  case AKW_DATA_TYPE_NUMBER:
    name = "Number";
    break;
  case AKW_DATA_TYPE_INT:
    name = "Int";
    break;
  case AKW_DATA_TYPE_STRING:
    name = "String";
    break;
  case AKW_DATA_TYPE_RANGE:
    name = "Range";
    break;
  case AKW_DATA_TYPE_ARRAY:
    name = "Array";
    break;
  }
  return name;
}

void akw_compiler_init(AkwCompiler *comp, int flags, char *source)
{
  comp->flags = flags;
  comp->rc = AKW_OK;
  akw_lexer_init(&comp->lex, source, &comp->rc, comp->err);
  if (!akw_compiler_is_ok(comp)) return;
  comp->scopeDepth = 0;
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

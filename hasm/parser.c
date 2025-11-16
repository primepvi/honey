#include "parser.h"
#include "../hvm/honey.h"
#include "../lib/sv.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct inst_info {
  const char* lexeme;
  inst_op_t op;
};

static struct inst_info NON_OPERAND_INSTS[] = {
    {"plusi", OP_PLUSI}, {"minusi", OP_MINUSI},
    {"divi", OP_DIVI},   {"multi", OP_MULTI},
    {"modi", OP_MODI},   {"gti", OP_GTI},
    {"gtei", OP_GTEI},   {"lti", OP_LTI},
    {"ltei", OP_LTEI},   {"eqi", OP_EQI},
    {"neqi", OP_NEQI},   {"noti", OP_NOTI},
    {"dump", OP_DUMP},   {"halt", OP_HALT},
};

static size_t NON_OPERAND_INSTS_COUNT = sizeof(NON_OPERAND_INSTS) / sizeof(struct inst_info);

parser_t *parser_new(token_t *tokens, size_t token_count) {
  parser_t *parser = malloc(sizeof(parser_t));
  parser->tokens = tokens;
  parser->token_count = token_count;
  
  parser->label_count = 0;
  parser->label_cap = 64;
  parser->labels = calloc(parser->label_cap, sizeof(label_t));

  parser->unresolved_addr_count = 0;
  parser->unresolved_addr_cap = 64;
  parser->unresolved_addrs = calloc(parser->unresolved_addr_cap, sizeof(label_t));
  
  parser->cursor = 0;

  return parser;
}

void parser_free(parser_t *parser) { free(parser); }

inst_t *parser_parse(parser_t *parser, size_t *out_size) {
  size_t inst_count = 0;
  size_t inst_cap = 32;
  inst_t *instructions = calloc(inst_cap, sizeof(inst_t));

  while (parser->cursor < parser->token_count) {
    inst_t current = parser_parse_inst(parser, inst_count);
    if (inst_count >= inst_cap) {
      inst_cap *= 2;
      instructions = realloc(instructions, inst_cap * sizeof(inst_t));
    }

    instructions[inst_count++] = current;
  }

  parser_resolve_addrs(parser, instructions, inst_count);

  *out_size = inst_count;
  return instructions;
}

inst_t parser_parse_inst(parser_t *parser, size_t inst_count) {
  token_t current = parser_consume(parser);

  while (current.kind == TOK_IDENTIFIER && parser_peek(parser).kind == TOK_COLON) {
    parser_expect(parser, TOK_COLON);
    parser_push_label(parser, current.lexeme, inst_count);
    current = parser_consume(parser);
  }
 
  if (sv_equals(current.lexeme, SV("push"))) {
    token_t operand = parser_expect(parser, TOK_NUMBER);
    const char *parsed_lexeme = sv_to_cstr(operand.lexeme);
    int64_t integer = atoi(parsed_lexeme);

    return (inst_t){.op = OP_PUSH, .operand = {.as_i64 = integer}};
  }

  if (sv_equals(current.lexeme, SV("dup"))) {
    token_t operand = parser_expect(parser, TOK_NUMBER);
    const char *parsed_lexeme = sv_to_cstr(operand.lexeme);
    uint64_t integer = atoi(parsed_lexeme);

    return (inst_t){.op = OP_DUP, .operand = {.as_u64 = integer}};
  }

  if (sv_equals(current.lexeme, SV("jmp")) ||
      sv_equals(current.lexeme, SV("jz")) ||
      sv_equals(current.lexeme, SV("jnz"))) {
    token_t operand = parser_peek(parser);
    inst_op_t op = sv_equals(current.lexeme, SV("jmp"))  ? OP_JMP
                   : sv_equals(current.lexeme, SV("jz")) ? OP_JZ
                     : OP_JNZ;
    
    if (operand.kind == TOK_NUMBER) {
      parser_expect(parser, TOK_NUMBER);
      const char *parsed_lexeme = sv_to_cstr(operand.lexeme);
      uint64_t integer = atoi(parsed_lexeme);

      return (inst_t){.op = op, .operand = {.as_i64 = integer}};
    } else {
      operand = parser_expect(parser, TOK_IDENTIFIER);
      parser_push_unresolved_addr(parser, operand.lexeme, inst_count);
      return (inst_t) {.op = op};
    }
  }

  for (size_t i = 0; i < NON_OPERAND_INSTS_COUNT; i++) {
    struct inst_info info = NON_OPERAND_INSTS[i];
    if (sv_equals(current.lexeme, SV(info.lexeme))) {
      return (inst_t) {.op = info.op };
    }
  }  
  
  fprintf(stderr, "parser (c: %zu) -> invalid instruction has found: '" SV_FMT "'\n",
          parser->cursor, SV_ARG(current.lexeme));
  exit(EXIT_FAILURE);
}

void parser_push_label(parser_t *parser, strview_t name, size_t ip) {
  if (parser->label_count >= parser->label_cap) {
    parser->label_cap *= 2;
    parser->labels = realloc(parser->labels, sizeof(label_t) * parser->label_cap);
  }

  parser->labels[parser->label_count++] = (label_t){.label = name, .ip = ip};
}

void parser_push_unresolved_addr(parser_t *parser, strview_t label, size_t ip) {
  if (parser->unresolved_addr_count >= parser->unresolved_addr_cap) {
    parser->unresolved_addr_cap *= 2;
    parser->unresolved_addrs = realloc(parser->unresolved_addrs, sizeof(label_t) * parser->unresolved_addr_cap);
  }

  parser->unresolved_addrs[parser->unresolved_addr_count++] = (label_t){.label = label, .ip = ip};
}

label_t parser_get_label(parser_t *parser, strview_t name) {
  for (size_t i = 0; i < parser->label_count; i++) {
    label_t current = parser->labels[i];
    if (sv_equals(current.label, name))
      return current;
  }

  fprintf(stderr, "parser -> " SV_FMT " is not a valid label.\n", SV_ARG(name));
  exit(EXIT_FAILURE);
}

void parser_resolve_addrs(parser_t *parser, inst_t *instructions, size_t inst_count) {
  for (size_t i = 0; i < parser->unresolved_addr_count; i++) {
    label_t unresolved = parser->unresolved_addrs[i];
    label_t label = parser_get_label(parser, unresolved.label);
    instructions[unresolved.ip].operand.as_u64 = label.ip;
  }
}


token_t parser_peek(parser_t *parser) { return parser->tokens[parser->cursor]; }
token_t parser_consume(parser_t *parser) { return parser->tokens[parser->cursor++]; }

token_t parser_expect(parser_t *parser, token_kind_t kind) {
  token_t current = parser_consume(parser);

  if (current.kind == kind)
    return current;

  if (current.kind == TOK_EOF) {
    fprintf(stderr, "parser -> expects %d, but received end of file.\n", kind);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "parser -> expects %d, but received %d.\n", kind,
          current.kind);
  exit(EXIT_FAILURE);
}

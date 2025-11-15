#include "parser.h"
#include "../hvm/honey.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

parser_t *parser_new(token_t *tokens, size_t token_count) {
  parser_t *parser = malloc(sizeof(parser_t));
  parser->tokens = tokens;
  parser->token_count = token_count;
  parser->cursor = 0;

  return parser;
}

void parser_free(parser_t *parser) { free(parser); }

inst_t *parser_parse(parser_t *parser, size_t *out_size) {
  size_t inst_count = 0;
  size_t inst_cap = 32;
  inst_t *instructions = calloc(inst_cap, sizeof(inst_t));

  while (parser->cursor < parser->token_count) {
    inst_t current = parser_parse_inst(parser);
    if (inst_count >= inst_cap) {
      inst_cap *= 2;
      instructions = realloc(instructions, inst_cap * sizeof(inst_t));
    }

    instructions[inst_count++] = current;
  }

  *out_size = inst_count;
  return instructions;
}

inst_t parser_parse_inst(parser_t *parser) {
  token_t current = parser_consume(parser);

  if (sv_equals(current.lexeme, SV("push"))) {
    token_t operand = parser_expect(parser, TOK_NUMBER);
    const char *parsed_lexeme = sv_to_cstr(operand.lexeme);
    int64_t integer = atoi(parsed_lexeme);
    
    return (inst_t) {
      .op = OP_PUSH, .operand = {
        .as_i64 = integer        
      }};
  }

  if (sv_equals(current.lexeme, SV("plusi"))){
    return (inst_t) { .op = OP_PLUSI };
  }

  if (sv_equals(current.lexeme, SV("minusi"))) {
    return (inst_t){.op = OP_MINUSI};
  }

  if (sv_equals(current.lexeme, SV("multi"))) {
    return (inst_t){.op = OP_MULTI};
  }

  if (sv_equals(current.lexeme, SV("divi"))) {
    return (inst_t){.op = OP_DIVI};
  }

  if (sv_equals(current.lexeme, SV("dump"))) {
    return (inst_t){.op = OP_DUMP};
  }

  if (sv_equals(current.lexeme, SV("halt"))) {
    return (inst_t){.op = OP_HALT};
  }

  fprintf(stderr, "parser -> invalid instruction has found: " SV_FMT "\n",
          SV_ARG(current.lexeme));
  exit(EXIT_FAILURE);
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

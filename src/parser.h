#pragma once

#include "lexer.h"
#include "sv.h"
#include "honey.h"
#include <stddef.h>

typedef struct {
  token_t *tokens;
  size_t token_count, cursor;
} parser_t;

parser_t *parser_new(token_t *tokens, size_t token_count);
void parser_free(parser_t *parser);

inst_t *parser_parse(parser_t *parser, size_t *out_size);
inst_t parser_parse_inst(parser_t *parser);

token_t parser_peek(parser_t *parser);
token_t parser_consume(parser_t *parser);
token_t parser_expect(parser_t *parser, token_kind_t kind);

#pragma once

#include "lexer.h"
#include "../lib/sv.h"
#include "../hvm/honey.h"
#include <stddef.h>

typedef struct {
  size_t ip;
  strview_t label;
} label_t;

typedef struct {
  token_t *tokens;
  size_t token_count;

  label_t *labels;
  size_t label_count, label_cap;

  label_t *unresolved_addrs;
  size_t unresolved_addr_count, unresolved_addr_cap;

  size_t cursor;
} parser_t;

parser_t *parser_new(token_t *tokens, size_t token_count);
void parser_free(parser_t *parser);

inst_t *parser_parse(parser_t *parser, size_t *out_size);
inst_t parser_parse_inst(parser_t *parser, size_t inst_count);

void parser_push_label(parser_t *parser, strview_t name, size_t ip);
void parser_push_unresolved_addr(parser_t *parser, strview_t label, size_t ip);
label_t parser_get_label(parser_t *parser, strview_t name);
void parser_resolve_addrs(parser_t *parser, inst_t *instructions, size_t inst_count);

token_t parser_peek(parser_t *parser);
token_t parser_consume(parser_t *parser);
token_t parser_expect(parser_t *parser, token_kind_t kind);

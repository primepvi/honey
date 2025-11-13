#define SV_IMPL
#include "sv.h"

#include "honey.h"
#include <assert.h>
#include <stdio.h>

#include "parser.h"

int main(void) {
  strview_t source = SV("# some commentary here\n# another comment\n\npush 10\n# comment here\npush 20\nplusi\ndump\nhalt");
  lexer_t *lexer = lexer_new(source);
  size_t token_count;
  token_t *tokens = lexer_lex(lexer, &token_count);

  for (size_t i = 0; i < token_count; i++) {
    token_t token = tokens[i];
    printf("k: %d, lexeme: "SV_FMT" \n", token.kind, SV_ARG(token.lexeme));
  }

  parser_t *parser = parser_new(tokens, token_count);
  size_t inst_count;
  inst_t *insts = parser_parse(parser, &inst_count);

  honey_t *hvm = honey_new(insts, inst_count);
  honey_interpret(hvm);
  
  return 0;
}

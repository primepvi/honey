#define SV_IMPL
#include "sv.h"

#include "honey.h"
#include <assert.h>
#include <stdio.h>
#include "lexer.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int main(void) {
  strview_t source = SV("# some commentary here\n# another comment\n\npush 10\n# comment here\npush 20\nplusi\ndebug:\ndump");
  lexer_t *lexer = lexer_new(source);
  size_t token_count;
  token_t *tokens= lexer_lex(lexer, &token_count);

  for (size_t i = 0; i < token_count; i++) {
    token_t token = tokens[i];
    printf("k: %d, lexeme: "SV_FMT" \n", token.kind, SV_ARG(token.lexeme));
  }

  return 0;
}

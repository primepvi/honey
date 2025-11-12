#include "sv.h"
#include <stddef.h>

typedef struct {
  strview_t buffer;
  size_t cursor;
} lexer_t;

typedef enum {
  TOK_IDENTIFIER,
  TOK_NUMBER,
  TOK_COLON,
  TOK_EOF
} token_kind_t;

typedef struct {
  token_kind_t kind;
  strview_t lexeme;
} token_t;

lexer_t *lexer_new(strview_t buffer);
void lexer_free(lexer_t *lexer);

token_t *lexer_lex(lexer_t *lexer, size_t *out_tokens_size);
token_t lexer_tokenize(lexer_t *lexer);

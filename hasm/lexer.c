#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static bool lexer_ident_predicate(char c) {
  return isalnum(c) || c == '_';
}

static bool lexer_number_predicate(char c) { return (bool)isdigit(c); }
static bool lexer_comment_predicate(char c) { return c != '\n'; }

lexer_t *lexer_new(strview_t buffer) {
  lexer_t *lexer = malloc(sizeof(lexer_t));
  lexer->buffer = buffer;
  lexer->cursor = 0;

  return lexer;
}

void lexer_free(lexer_t *lexer) { free(lexer); }

token_t *lexer_lex(lexer_t *lexer, size_t *out_tokens_size) {
  size_t token_count = 0;
  size_t token_cap = 25;
  token_t *tokens = (token_t*) calloc(token_cap, sizeof(token_t));

  while (lexer->cursor < lexer->buffer.length) {
    token_t current = lexer_tokenize(lexer);
    if (token_count >= token_cap) {
      token_cap *= 2;
      tokens = (token_t*) realloc(tokens, sizeof(token_t) * token_cap);
    }

    tokens[token_count++] = current;
  }

  *out_tokens_size = token_count;
  return tokens;
}

token_t lexer_tokenize(lexer_t *lexer) {
  strview_t source = sv_slice(lexer->buffer, lexer->cursor, SV_END);

  size_t spaces = sv_ltrim(&source);
  lexer->cursor += spaces;
  
  while (sv_starts_with(source, SV("#"))) {
    strview_t comment = sv_take_while(source, lexer_comment_predicate);
    sv_ldrop(&source, comment.length);
    spaces = sv_ltrim(&source);
    lexer->cursor += comment.length + spaces;
  }
  
  if (source.length <= 0) {
    return (token_t){.kind = TOK_EOF, .lexeme = SV("\0")};
  }    

  strview_t number = sv_take_while(source, lexer_number_predicate);
  if (number.length > 0) {
    lexer->cursor += number.length;
    return (token_t){.kind = TOK_NUMBER, .lexeme = number};
  }

  strview_t identifier = sv_take_while(source, lexer_ident_predicate);
  if (identifier.length > 0) {
    lexer->cursor += identifier.length;
    return (token_t){.kind = TOK_IDENTIFIER, .lexeme = identifier};
  }    

  lexer->cursor++;
  if (sv_starts_with(source, SV(":")))
    return (token_t){.kind = TOK_COLON, .lexeme = SV(":")};

  fprintf(stderr, "lexer (c: %zu) -> invalid token has found: %c", lexer->cursor, source.buffer[0]);
  exit(EXIT_FAILURE);
}

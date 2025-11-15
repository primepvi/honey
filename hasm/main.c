#define SV_IMPL
#include "../lib/sv.h"

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

char *load_file_buffer(const char *filepath, size_t *out_size) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "error -> invalid input filepath.\n");
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  *out_size = ftell(file);
  rewind(file);

  char *buffer = malloc(sizeof(char) * (*out_size + 1));
  if (!buffer) {
    fprintf(stderr, "error -> cannot alloc memory to file buffer.\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fread(buffer, sizeof(char), *out_size, file);
  buffer[*out_size] = '\0';
  fclose(file);

  return buffer;
}

void write_file_bytecode(const char *filepath, inst_t *instructions,
                         size_t inst_count) {
  FILE *output = fopen(filepath, "wb");
  if (!output) {
    fprintf(stderr, "error -> cannot open output file.\n");
    exit(EXIT_FAILURE);
  }

  size_t write_count = fwrite(instructions, sizeof(inst_t), inst_count, output);
  if (write_count != inst_count) {
    fprintf(stderr, "error -> unexpected error during output file writing.\n");
    fclose(output);
    exit(EXIT_FAILURE);
  }

  fclose(output);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("error -> invalid usage.\n");
    printf("Usage: hasm <input> <output>\n"); 
    return EXIT_FAILURE;
  }

  char *input_path = argv[1];
  char *output_path = argv[2];

  size_t source_code_len;
  char *source_code = load_file_buffer(input_path, &source_code_len);

  lexer_t *lexer = lexer_new(sv_create(source_code, source_code_len));
  size_t token_count;
  token_t *tokens = lexer_lex(lexer, &token_count);

  parser_t *parser = parser_new(tokens, token_count);
  size_t inst_count;
  inst_t *instructions = parser_parse(parser, &inst_count);

  write_file_bytecode(output_path, instructions, inst_count);

  lexer_free(lexer);
  parser_free(parser);
  free(source_code);
  free(tokens);
  free(instructions);

  return 0;
}

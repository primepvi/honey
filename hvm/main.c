#define SV_IMPL
#include "../lib/sv.h"

#include "honey.h"
#include <stdio.h>
#include <stdlib.h>

inst_t *load_bytecode_file(const char *filepath, size_t *out_count) {
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "error -> invalid input filepath.\n");
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  *out_count = size / sizeof(inst_t);
  inst_t *program = malloc(sizeof(inst_t) * *out_count);
  if (!program) {
    fprintf(stderr, "error -> cannot alloc memory to program.\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fread(program, sizeof(inst_t), *out_count, file);
  fclose(file);

  return program;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("error -> invalid usage.\n");
    printf("Usage: hvm <input>\n");
    return EXIT_FAILURE;
  }

  char *input_path = argv[1];
  size_t inst_count;
  inst_t *program = load_bytecode_file(input_path, &inst_count);

  honey_t *hvm = honey_new(program, inst_count);
  honey_interpret(hvm);

  honey_free(hvm);
  free(program);

  return 0;
}

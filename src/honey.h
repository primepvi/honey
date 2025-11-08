#pragma once

#include <stddef.h>
#include <stdint.h>

#define STACK_MAX 1024

typedef int64_t word_t;

typedef enum inst_op {
  OP_PUSH,
  OP_PLUS,
  OP_DUMP,
  OP_HALT,
} inst_op_t;

typedef struct inst {
  inst_op_t op;
  int32_t a, b;
} inst_t;

#define MK_INST_PUSH(val)                                                      \
  (inst_t) { .op = OP_PUSH, .a = (val) }
#define MK_INST_PLUS                                                           \
  (inst_t) { .op = OP_PLUS }
#define MK_INST_DUMP                                                           \
  (inst_t) { .op = OP_DUMP }
#define MK_INST_HALT                                                           \
  (inst_t) { .op = OP_HALT }

typedef enum err_code {
  ERR_OK = 0,
  ERR_STACK_UNDERFLOW,
  ERR_STACK_OVERFLOW,
  ERR_INST_ILLEGAL_ACCESS,
} err_code_t;

typedef struct honey {
  inst_t *program;
  size_t program_size;

  word_t stack[STACK_MAX];
  size_t sp, ip;
} honey_t;

honey_t *honey_new(inst_t *program, size_t program_size);
void honey_free(honey_t *vm);

err_code_t honey_stack_push(honey_t *vm, word_t value);
err_code_t honey_stack_pop(honey_t *vm, word_t *out);

const char *honey_error_cstr(err_code_t code);
const char *honey_inst_cstr(inst_op_t op);

void honey_stack_dump(const honey_t *vm);
void honey_panic(const honey_t *vm, err_code_t code, const inst_t *current);

err_code_t honey_interpret(honey_t *vm);

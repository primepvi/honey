#pragma once

#include <stddef.h>
#include <stdint.h>

#define STACK_MAX 1024

typedef struct word {
  union {
    int64_t as_i64;
    uint64_t as_u64;
    double as_f64;
    void *as_ptr;
  };
} word_t;

typedef enum inst_op {
  OP_PUSH,

  OP_PLUSI,
  OP_MINUSI,
  OP_DIVI,
  OP_MULTI,
  OP_MODI,
  
  OP_GTI,
  OP_GTEI,
  OP_LTI,
  OP_LTEI,
  OP_EQI,
  OP_NEQI,
  OP_NOTI,

  OP_JMP,
  OP_JZ,
  OP_JNZ,
  
  OP_DUP,
  OP_DUMP,
  OP_HALT,
} inst_op_t;

typedef struct inst {
  inst_op_t op;
  word_t operand;
} inst_t;

#define BINARY_OPI(op)                                                         \
  {                                                                            \
    word_t a, b;                                                               \
    err_code_t res_a = honey_stack_pop(vm, &a);                                \
    err_code_t res_b = honey_stack_pop(vm, &b);                                \
    if (res_a != ERR_OK || res_b != ERR_OK) {                                  \
      err_code_t err = res_a != ERR_OK ? res_a : res_b;                        \
      honey_panic(vm, err, &current);                                          \
      return err;                                                              \
    }                                                                          \
    err_code_t push_res =                                                      \
        honey_stack_push(vm, (word_t){.as_i64 = b.as_i64 op a.as_i64});        \
    if (push_res != ERR_OK) {                                                  \
      honey_panic(vm, push_res, &current);                                     \
      return push_res;                                                         \
    }                                                                          \
    break;                                                                     \
  }

typedef enum err_code {
  ERR_OK = 0,
  ERR_STACK_UNDERFLOW,
  ERR_STACK_OVERFLOW,
  ERR_STACK_ILLEGAL_ACCESS,
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

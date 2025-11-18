#include "honey.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BINARY_OPI(op)                                                         \
  {                                                                         \
    word_t a, b;                                                               \
    err_code_t res_a = honey_stack_pop(vm, &a);                                \
    err_code_t res_b = honey_stack_pop(vm, &b);                                \
    PANIC_ASSERT(vm, res_a, current);                                          \
    PANIC_ASSERT(vm, res_b, current);                                          \
    err_code_t push_res =                                                      \
        honey_stack_push(vm, (word_t){.as_i64 = b.as_i64 op a.as_i64});        \
    PANIC_ASSERT(vm, push_res, current);                                       \
    break;                                                                     \
  }

#define PANIC_ASSERT(vm, res, inst)                                            \
  do {                                                                         \
    if (res != ERR_OK) {                                                       \
      honey_panic(vm, res, &inst);                                             \
      return res;                                                              \
    }                                                                          \
  } while (false)

honey_t *honey_new(inst_t *program, size_t program_size) {
  honey_t *vm = calloc(1, sizeof(honey_t));
  if (!vm)
    return NULL;

  vm->program = program;
  vm->program_size = program_size;
  return vm;
}

void honey_free(honey_t *vm) { free(vm); }

err_code_t honey_stack_push(honey_t *vm, word_t value) {
  if (vm->sp >= STACK_MAX)
    return ERR_STACK_OVERFLOW;

  vm->stack[vm->sp++] = value;
  return ERR_OK;
}

err_code_t honey_stack_pop(honey_t *vm, word_t *out) {
  if (vm->sp == 0)
    return ERR_STACK_UNDERFLOW;

  *out = vm->stack[--vm->sp];
  return ERR_OK;
}

const char *honey_error_cstr(err_code_t code) {
  switch (code) {
  case ERR_OK:
    return "No error";
  case ERR_STACK_OVERFLOW:
    return "Stack overflow: exceeded maximum stack size";
  case ERR_STACK_UNDERFLOW:
    return "Stack underflow: attempted to pop from empty stack";
  case ERR_STACK_ILLEGAL_ACCESS:
    return "Illegal stack access out of bounds";
  case ERR_INST_ILLEGAL_ACCESS:
    return "Illegal program memory access: out-of-bounds read or jump";
  default:
    return "Unknown error.";
  }
}

void honey_stack_dump(const honey_t *vm) {
  printf("Stack:\n");

  if (vm->sp <= 0) {
    printf("  [ empty ]\n");
  } else {
    for (size_t i = 0; i < vm->sp; i++) {
      word_t word = vm->stack[i];
      printf("  i64: %ld, u64: %lu, f64: %lf, ptr: %p\n", word.as_i64,
             word.as_u64, word.as_f64, word.as_ptr);
    }
  }
}

void honey_panic(const honey_t *vm, err_code_t code, const inst_t *current) {
  fprintf(stderr, "\n[VM ERROR] %s\n", honey_error_cstr(code));

  if (current)
    fprintf(stderr, "  -> Instruction: (operand=%ld)\n",
            current->operand.as_i64);

  fprintf(stderr, "  IP=%zu, SP=%zu\n", vm->ip, vm->sp);

  honey_stack_dump(vm);
  fprintf(stderr, "\n");
}

err_code_t honey_interpret(honey_t *vm) {
  while (1) {
    if (vm->ip >= vm->program_size) {
      honey_panic(vm, ERR_INST_ILLEGAL_ACCESS, NULL);
      return ERR_INST_ILLEGAL_ACCESS;
    }

    inst_t current = vm->program[vm->ip++];
    switch (current.op) {
    case OP_PUSH: {
      err_code_t res = honey_stack_push(vm, current.operand);
      PANIC_ASSERT(vm, res, current);
      break;
    }
    case OP_PLUSI: BINARY_OPI(+);
    case OP_MINUSI: BINARY_OPI(-);
    case OP_DIVI: BINARY_OPI(/);
    case OP_MULTI: BINARY_OPI(*);
    case OP_MODI: BINARY_OPI(%);
    case OP_LTI: BINARY_OPI(<);
    case OP_LTEI: BINARY_OPI(<=);
    case OP_GTI: BINARY_OPI(>);
    case OP_GTEI: BINARY_OPI(>=);
    case OP_EQI: BINARY_OPI(==);
    case OP_NEQI: BINARY_OPI(!=);
    case OP_NOTI: {
      word_t word;
      err_code_t res = honey_stack_pop(vm, &word);
      PANIC_ASSERT(vm, res, current);

      word.as_i64 = !word.as_i64;
      res = honey_stack_push(vm, word);
      PANIC_ASSERT(vm, res, current);

      break;
    }
    case OP_DUP: {
      if (current.operand.as_u64 >= vm->sp) {
        honey_panic(vm, ERR_STACK_ILLEGAL_ACCESS, &current);
        return ERR_STACK_ILLEGAL_ACCESS;
      }

      word_t word = vm->stack[current.operand.as_u64];
      err_code_t res = honey_stack_push(vm, word);
      PANIC_ASSERT(vm, res, current);
      
      break;
    }
    case OP_DUMP: {
      word_t word;
      err_code_t res = honey_stack_pop(vm, &word);
      PANIC_ASSERT(vm, res, current);
      
      printf("  i64: %ld, u64: %lu, f64: %lf, ptr: %p\n", word.as_i64,
             word.as_u64, word.as_f64, word.as_ptr);
      break;
    }
    case OP_JMP: {
      size_t target = current.operand.as_u64;
      if (target >= vm->program_size) {
        honey_panic(vm, ERR_INST_ILLEGAL_ACCESS, &current);
        return ERR_INST_ILLEGAL_ACCESS;
      }
      
      vm->ip = target;
      break;
    }
    case OP_JZ: {
      word_t word;
      err_code_t res = honey_stack_pop(vm, &word);
      PANIC_ASSERT(vm, res, current);

      if (word.as_i64 == 0) {
        size_t target = current.operand.as_u64;
        if (target >= vm->program_size) {
          honey_panic(vm, ERR_INST_ILLEGAL_ACCESS, &current);
          return ERR_INST_ILLEGAL_ACCESS;
        }

        vm->ip = target;
      }
      
      break;
    }
    case OP_JNZ: {
      word_t word;
      err_code_t res = honey_stack_pop(vm, &word);
      PANIC_ASSERT(vm, res, current);

      if (word.as_i64 != 0) {
        size_t target = current.operand.as_u64;
        if (target >= vm->program_size) {
          honey_panic(vm, ERR_INST_ILLEGAL_ACCESS, &current);
          return ERR_INST_ILLEGAL_ACCESS;
        }

        vm->ip = target;
      }

      break;
    }
    case OP_HALT:
      return ERR_OK;
    default: {
      fprintf(stderr, "err: Unimplemented instruction has found -> %d\n",
              current.op);
      exit(EXIT_FAILURE);
    }
    }
  };

  return ERR_OK;
}

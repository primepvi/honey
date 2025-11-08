#include "honey.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
  if (vm->sp <= 0)
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
  case ERR_INST_ILLEGAL_ACCESS:
    return "Illegal program memory access: out-of-bounds read or jump";
  default:
    return "Unknown error.";
  }
}

const char *honey_inst_cstr(inst_op_t op) {
  switch (op) {
  case OP_PUSH:
    return "OP_PUSH";
  case OP_DUMP:
    return "OP_DUMP";
  case OP_HALT:
    return "OP_HALT";
  case OP_PLUS:
    return "OP_PLUS";
  default:
    fprintf(stderr, "unexpected: unknown instruction in honey_inst_cstr: %d\n",
            op);
    exit(EXIT_FAILURE);
  }
}

void honey_stack_dump(const honey_t *vm) {
  printf("Stack:\n");

  if (vm->sp <= 0) {
    printf("  [ empty ]\n");
  } else {
    for (size_t i = 0; i < vm->sp; i++)
      printf("  %ld\n", vm->stack[i]);
  }
}

void honey_panic(const honey_t *vm, err_code_t code, const inst_t *current) {
  fprintf(stderr, "\n[VM ERROR] %s\n", honey_error_cstr(code));

  if (current)
    fprintf(stderr, "  -> Instruction: %s (a=%d, b=%d)\n",
            honey_inst_cstr(current->op), current->a, current->b);

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
      err_code_t res = honey_stack_push(vm, current.a);
      if (res != ERR_OK) {
        honey_panic(vm, res, &current);
        return res;
      }

      break;
    }
    case OP_PLUS: {
      word_t a, b;
      err_code_t res_a = honey_stack_pop(vm, &a);
      err_code_t res_b = honey_stack_pop(vm, &b);
      if (res_a != ERR_OK || res_b != ERR_OK) {
        err_code_t err = res_a != ERR_OK ? res_a : res_b;
        honey_panic(vm, err, &current);
        return err;
      }

      err_code_t push_res = honey_stack_push(vm, a + b);
      if (push_res != ERR_OK) {
        honey_panic(vm, push_res, &current);
        return push_res;
      }

      break;
    }
    case OP_DUMP: {
      word_t value;
      err_code_t res = honey_stack_pop(vm, &value);
      if (res != ERR_OK) {
        honey_panic(vm, res, &current);
        return res;
      }

      printf("%ld\n", value);
      break;
    }
    case OP_HALT:
      return ERR_OK;

    default: {
      fprintf(stderr, "err: Unimplemented instruction has found -> %d:%s\n",
              current.op, honey_inst_cstr(current.op));
      exit(EXIT_FAILURE);
    }
    }
  };

  return ERR_OK;
}

#include "honey.h"
#include <assert.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int main(void) {
  inst_t program[] = {
      MK_INST_PUSH(34), MK_INST_PUSH(35), MK_INST_PLUS,
      MK_INST_DUMP,     MK_INST_HALT,
  };

  honey_t *honey = honey_new(program, ARRAY_SIZE(program));
  err_code_t res = honey_interpret(honey);
  if (res == ERR_OK)
    honey_stack_dump(honey);

  honey_inst_cstr(OP_PLUS);

  honey_free(honey);

  return 0;
}

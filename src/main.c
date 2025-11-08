#include "honey.h"
#include <assert.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int main(void) {
  inst_t program[] = {
      MK_INST_PUSH(10), MK_INST_PUSH(20), MK_INST_PUSH(10),
      MK_INST_PLUSI,    MK_INST_MULTI,    MK_INST_HALT,
  };

  honey_t *honey = honey_new(program, ARRAY_SIZE(program));
  err_code_t res = honey_interpret(honey);
  if (res == ERR_OK)
    honey_stack_dump(honey);

  honey_free(honey);

  return 0;
}

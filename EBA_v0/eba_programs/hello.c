#include "eba_common.h"


void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("hello!\n");

   op_loader_t *br_opl = opl_init("eba_core/bufreq.so", "bufreq");
   void *br_arg = init_eba_arg(1);
   set_eba_arg(br_arg, 0, br_opl);
   eba_op(br_arg);
   free_eba_arg(br_arg);
   opl_destroy(br_opl);

}

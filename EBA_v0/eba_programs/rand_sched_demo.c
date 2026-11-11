// random sched demo

#include "eba_common.h"
#include "fifo_sched_common.h"

#include <stdlib.h> // for srand & rand

// creates a randomized scheduler with no interrupts and unlimited wait queue.
// Uses EBA features to manage its memory during its lifespan

#define DEMO_LEN 100
void rand_print(void *args);

void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("scheduler demo: writing sequential numbers from 0 to %d to a randomized scheduler.\n", DEMO_LEN-1);

   // checking an assumption
   if (sizeof(void*) < sizeof(FILE*))
   {
      printf("ERROR: pointer size assumptions not met. Aborting.\n");
      return;
   }

   // SCHEDULER INIT
   op_loader_t *fs_main_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_run_random");
   op_loader_t *fs_init_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_init");

   void *fs_main_arg = init_eba_arg(2);
   void *fs_init_arg = init_eba_arg(2);

   set_eba_arg(fs_main_arg, 0, fs_main_opl);
   set_eba_arg(fs_init_arg, 0, fs_init_opl);

   eba_op(fs_init_arg);

   fifo_sched_t *fs = get_eba_arg(fs_init_arg, 1);
   set_eba_arg(fs_main_arg, 1, fs);


   // set up the prints
   op_loader_t *print_opl = opl_init("eba_programs/rand_sched_demo.so", "rand_print");

   uint64_t i;
   for(i = 0; i < DEMO_LEN; i++)
   {
      void *print_arg = init_eba_arg(2);
      set_eba_arg(print_arg, 0, print_opl);
      set_eba_arg(print_arg, 1, (void*)i);
      void *add_to_sched_args = init_eba_arg(3);
      set_eba_arg(add_to_sched_args, 0, fs->add_opl);
      set_eba_arg(add_to_sched_args, 1, fs);
      set_eba_arg(add_to_sched_args, 2, print_arg);
      eba_op(add_to_sched_args);
   }


   // set up random seed
   // (guaranteeing the same result every time on a given seed)
   srand(1);


   // RUN
   eba_op(fs_main_arg);

   // CLEANUP
   opl_destroy(fs_main_opl);
   opl_destroy(fs_init_opl);
   opl_destroy(print_opl);
   free_eba_arg(fs_main_arg);
   free_eba_arg(fs_init_arg);
}



void rand_print(void *args)
{
   uint64_t num_to_print = (uint64_t)get_eba_arg(args, 1);
   printf("%lu\n", num_to_print);
   free_eba_arg(args);
}

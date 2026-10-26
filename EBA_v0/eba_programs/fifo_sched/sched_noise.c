#include "eba_common.h"
#include "fifo_sched_common.h"

#define PASS_LENGTH 100
#define NOISE_CAP 100

// a noise program that is meant to duplicate itself several times,
// possibly using `rand()`
void sched_noise(void* args)
{
   // arguments:
   // 0: this operation
   // 1: the scheduler
   // 2: which iteration this operation is on (buffer containing the iteration)
   // 3: the assignment 2 thread/continuation-chain ID

   int64_t *iterp = (int64_t*)get_eba_arg(args, 2);
   int64_t iter = *iterp;
   free(iterp);
   fifo_sched_t *fs = get_eba_arg(args, 1);
   uintptr_t thread_id = (uintptr_t)get_eba_arg(args, 3);


   int i;
   int new_progs = 0;
   // technically, this is not even fully pseudorandom.
   // there is some rounding error near RAND_MAX since there exists many
   // X such that RAND_MAX%X!=0.
   // for this pedagogical example, we will ignore this.

   // for a highly noisy environment that may have many, or very few operations.
   if (iter < PASS_LENGTH)
   {
      // imagine just rolling two dice to keep the noise slightly lower
      int r1 = rand()%NOISE_CAP;
      int r2 = rand()%NOISE_CAP;
      new_progs = 1+r1+r2;
   }

   for(i = 0; i < new_progs; i++)
   {
      // Assignment 2: reserve one extra argument for the chain ID.
      // The first child stays on this chain; later children begin new chains.
      void *my_next_iter = init_eba_arg(4);
      set_eba_arg(my_next_iter, 0, get_eba_arg(args, 0));
      set_eba_arg(my_next_iter, 1, fs);
      int64_t *next_iterp = malloc(sizeof(int64_t));
      *next_iterp = iter+1; // only let the first one reproduce
      *next_iterp += (i>0)*PASS_LENGTH; // others will have iter being too high.
      set_eba_arg(my_next_iter, 2, next_iterp);
      if (i == 0)
      {
         set_eba_arg(my_next_iter, 3, (void *)thread_id);
      }
      else
      {
         set_eba_arg(my_next_iter, 3, (void *)(uintptr_t)fs->next_thread_id++);
      }


      // now put it on the scheduler
      void *add_to_sched_args = init_eba_arg(3);
      set_eba_arg(add_to_sched_args, 0, fs->add_opl);
      set_eba_arg(add_to_sched_args, 1, fs);
      set_eba_arg(add_to_sched_args, 2, my_next_iter);

      // then duplicate this
      eba_op(add_to_sched_args);
   }



   free_eba_arg(args);
}

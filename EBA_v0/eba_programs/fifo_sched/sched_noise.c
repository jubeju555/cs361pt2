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

   int64_t *iterp = (int64_t*)get_eba_arg(args, 2);
   int64_t iter = *iterp;
   free(iterp);
   fifo_sched_t *fs = get_eba_arg(args, 1);


   int i;
   int new_progs = 0;
   int do_logistic = 0;
   // technically, this is not even fully pseudorandom.
   // there is some rounding error near RAND_MAX since there exists many
   // X such that RAND_MAX%X!=0.
   // for this pedagogical example, we will ignore this.
   if (do_logistic)
   {
      int r1 = rand()%PASS_LENGTH;
      int r2 = rand()%PASS_LENGTH;
      // for a logisticly scaling and decreasing "spike" of program activity
      if (r1*2 > iter-PASS_LENGTH/2 && iter < PASS_LENGTH)
      {
         // For the first PASS_LENGTH/2 iterations, always respawn. After that,
         // a growing chance not to respawn eventually reaching 25 when iter=PASS_LENGTH-1
         // and always disallowed after PASSLENGTH
         new_progs += 1;
         // printf("Yes: we should respawn\n");
      }
      if (r2 >= iter+PASS_LENGTH/2)
      {
         // a chance to duplicate the program, starting at 50% and decreasing to 0%
         new_progs += 1;
         // printf("Yes: we should duplicate\n");
      }
   }
   else
   {
      // for a highly noisy environment that may have many, or very few operations.
      if (iter < PASS_LENGTH)
      {
         int r1 = rand()%NOISE_CAP;
         int r2 = rand()%NOISE_CAP;

         new_progs = 1+r1+r2;
      }
   }

   for(i = 0; i < new_progs; i++)
   {
      // build an eba arg that we'll be adding
      void *my_next_iter = init_eba_arg(3);
      set_eba_arg(my_next_iter, 0, get_eba_arg(args, 0));
      set_eba_arg(my_next_iter, 1, fs);
      int64_t *next_iterp = malloc(sizeof(int64_t));
      *next_iterp = iter+1;
      if (!do_logistic)
      {
         *next_iterp += (i>0)*PASS_LENGTH;
      }
      set_eba_arg(my_next_iter, 2, next_iterp);


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

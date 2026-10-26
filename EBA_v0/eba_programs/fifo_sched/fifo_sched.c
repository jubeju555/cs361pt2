#include "eba_common.h"
#include "fifo_sched_common.h"

#include <sys/time.h>

// creates a FIFO scheduler with no interrupts and unlimited wait queue.

// options: will we do A or B?
// A: requires all operations that are called to have a "next" field, which
// this will set to point back to itself
// B: requires EBA_run or some other function that will always run, and the
// next op that it runs will be some kind of global or settable variable.
// Perhaps it is passed to called funcitons
//
// option B is the better option, as option A would result in arbitrarily
// long call depth when implemented using C.



void sched_init(void *args)
{
   // args are:
   // 0: this operation
   // 1: where I will place the scheduler
   fifo_sched_t *fs = malloc(sizeof(fifo_sched_t));

   fs->arg_block_next = calloc(SCHED_BUF_LENGTH, sizeof(void*));
   fs->arg_block_end = fs->arg_block_next;
   fs->next_idx = 0;
   fs->end_idx = 0;
   // Assignment 2: initialize the queue counter, thread IDs, and event log.
   fs->queue_length = 0;
   fs->next_thread_id = 1;
   fs->schedule_log = NULL;
   fs->add_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "add_to_sched");
   fs->last_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "last_entry_in_buf");
   
   void *new_buf_last_entry = init_eba_arg(4);
   set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
   set_eba_arg(new_buf_last_entry, 1, fs);
   set_eba_arg(new_buf_last_entry, 2, fs->arg_block_next);
   set_eba_arg(new_buf_last_entry, 3, NULL);

   fs->arg_block_next[SCHED_BUF_LENGTH-1] = new_buf_last_entry;

   set_eba_arg(args, 1, fs);
}

void last_entry_in_buf(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to free
   // 3: the next arg_block_next pointer (might be NULL)
   fifo_sched_t *fs = get_eba_arg(args, 1);

   free(get_eba_arg(args, 2));

   fs->arg_block_next = get_eba_arg(args, 3);

   fs->next_idx = 0;

   free_eba_arg(args);
}

void add_to_sched(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to add to the scheduler
   fifo_sched_t *fs = get_eba_arg(args, 1);
   void *add_me = get_eba_arg(args, 2);

   // Assignment 2: reserve a new block before rebuilding the randomized
   // queue when the current final block has no data slot left.
   if (fs->end_idx == SCHED_BUF_LENGTH-1)
   {
      void *last_entry_arg_buf = fs->arg_block_end[fs->end_idx];
      void **new_buf = calloc(SCHED_BUF_LENGTH,sizeof(void*));
      set_eba_arg(last_entry_arg_buf, 3, new_buf);

      fs->arg_block_end = new_buf;
      fs->end_idx = 0;

      void *new_buf_last_entry = init_eba_arg(4);
      set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
      set_eba_arg(new_buf_last_entry, 1, fs);
      set_eba_arg(new_buf_last_entry, 2, new_buf);
      set_eba_arg(new_buf_last_entry, 3, NULL);
      fs->arg_block_end[SCHED_BUF_LENGTH-1] = new_buf_last_entry;
   }

   // Assignment 2: flatten the linked queue, insert at a random position,
   // and write the reordered continuations back into the existing blocks.
   uint64_t old_length = fs->queue_length;
   void **queued = malloc((old_length + 1) * sizeof(void *));
   void **arg_block = fs->arg_block_next;
   uint64_t block_idx = fs->next_idx;
   uint64_t queued_idx = 0;

   while (arg_block != fs->arg_block_end)
   {
      while (block_idx < SCHED_BUF_LENGTH-1)
      {
         queued[queued_idx++] = arg_block[block_idx++];
      }
      arg_block = get_eba_arg(arg_block[SCHED_BUF_LENGTH-1], 3);
      block_idx = 0;
   }
   while (block_idx < fs->end_idx)
   {
      queued[queued_idx++] = arg_block[block_idx++];
   }

   uint64_t insert_idx = rand() % (old_length + 1);
   for (uint64_t i = old_length; i > insert_idx; --i)
   {
      queued[i] = queued[i-1];
   }
   queued[insert_idx] = add_me;

   arg_block = fs->arg_block_next;
   block_idx = fs->next_idx;
   queued_idx = 0;
   while (arg_block != fs->arg_block_end)
   {
      while (block_idx < SCHED_BUF_LENGTH-1)
      {
         arg_block[block_idx++] = queued[queued_idx++];
      }
      arg_block = get_eba_arg(arg_block[SCHED_BUF_LENGTH-1], 3);
      block_idx = 0;
   }
   while (queued_idx < old_length + 1)
   {
      arg_block[block_idx++] = queued[queued_idx++];
   }
   fs->end_idx = block_idx;
   fs->queue_length += 1;

   free(queued);
   free_eba_arg(args);
}

void sched_run(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   fifo_sched_t *fs = get_eba_arg(args, 1);

   while(1)
   {
      // printf("running with next=%ld\n", fs->next_idx);
      if (fs->arg_block_next == NULL)
      {
         if (fs->schedule_log != NULL)
         {
            fclose(fs->schedule_log);
         }
         opl_destroy(fs->add_opl);
         opl_destroy(fs->last_opl);
         free(fs);
         printf("scheduler exit: success\n");
         return;
      }

      // else:
      // Assignment 2: dequeue one continuation and record its thread ID.
      uint64_t current_idx = fs->next_idx;
      void *next_eba_op = fs->arg_block_next[current_idx];
      fs->next_idx += 1;
      if (next_eba_op != NULL)
      {
         if (current_idx != SCHED_BUF_LENGTH-1)
         {
            fs->queue_length -= 1;
            if (fs->schedule_log != NULL)
            {
               struct timeval now;
               gettimeofday(&now, NULL);
               uintptr_t thread_id =
                  (uintptr_t)get_eba_arg(next_eba_op, 3);
               fprintf(fs->schedule_log, "%ld.%06ld,%lu\n",
                       (long)now.tv_sec, (long)now.tv_usec,
                       (unsigned long)thread_id);
               fflush(fs->schedule_log);
            }
         }
         eba_op(next_eba_op);
      }
      else
      {
         ; // pass
      }
   }
}

// for debugging. NOT an EBA arg.
void sched_print(fifo_sched_t *fs)
{
   printf("SCHEDULER:\n");
   printf("next buf: 0x%lx\n", (uint64_t)fs->arg_bufs_buf_next);
   printf("end buf : 0x%lx\n", (uint64_t)fs->arg_bufs_buf_end);
   printf("next idx: %ld\n", fs->next_idx);
   printf("end idx : %ld\n", fs->end_idx);
   printf("contents:\n");
   void **which_bufs_buf;
   int num_iters = 0;
   for(which_bufs_buf = fs->arg_bufs_buf_next; which_bufs_buf != NULL; )
   {
      printf("   showing bufs buf 0x%lx:\n", (uint64_t)which_bufs_buf);
      num_iters += 1;
      int i;
      for(i = 0; i < SCHED_BUF_LENGTH; i++)
      {
         if(which_bufs_buf == fs->arg_bufs_buf_next && i == fs->next_idx)
         {
            printf(" * ");
         }
         else
         {
            printf("   ");
         }
         if(which_bufs_buf == fs->arg_bufs_buf_end && i == fs->end_idx)
         {
            printf(" * ");
         }
         else
         {
            printf("   ");
         }
         printf("%03d: 0x%016lx\n", i, (uint64_t)which_bufs_buf[i]);
      }
      void *final_arg = which_bufs_buf[SCHED_BUF_LENGTH-1];
      which_bufs_buf = (void**)get_eba_arg(final_arg, 3);
   }
   // the number of extra iterations will be (SCHED_BUF_LENGTH-1) extra entries
   int sched_len = (num_iters-1)*(SCHED_BUF_LENGTH-1) + fs->end_idx - fs->next_idx;
   printf("sched_len: %d\n", sched_len);
}

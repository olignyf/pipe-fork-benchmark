/*	Copyright 2012 Christoph Gärtner, ooxi/entities
		https://bitbucket.org/cggaertner/cstuff
		https://github.com/ooxi/entities

	Distributed under the Boost Software License, Version 1.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#undef NDEBUG
#include <assert.h>

int main(void)
{
  int64_t compute = 1;
  unsigned int i = 0;
  while (i < 10000000)
  {
    i++;
    compute += rand() * rand();
    printf("! %d doing stuff %lld !\n", i, compute);
//    if (i % 10 == 0) fprintf(stderr, "!err: oups %d !\n", i);
  }
  
  printf("end of printer\n");
  fflush(stdout);

  return EXIT_SUCCESS;
}


/*
         unsigned int i;
         const char * lookup = stderr_buf;
         unsigned int lookup_len = sizeof(stderr_buf);
         for (i=0; i<lookup_len; i++)
         {
            if (lookup[i])
            {
              printf("STDERR buffer was used\n");
              break;
            }
         }
         if (i <= lookup_len)
         {
            printf("STDERR buffer was UNUSED\n");
         }
         
         lookup = stdout_buf;
         lookup_len = sizeof(stdout_buf);
         for (i=0; i<lookup_len; i++)
         {
            if (lookup[i])
            {
              printf("STDOUT buffer was used\n");
              break;
            }
         }
         if (i <= lookup_len)
         {
            printf("STDOUT buffer was UNUSED\n");
         }
*/
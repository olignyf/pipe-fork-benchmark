/*
	Copyright 2019 Francois Oligny-Lemieux
	https://github.com/olignyf

	Distributed under the Boost Software License, Version 1.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "entities.h"

#undef NDEBUG
#include <assert.h>

int main(void)
{
  int64_t compute = 1;
  unsigned int i = 0;
  
  // Put buffering on STDERR to prevent performance hit
#if 0
  static char stderr_buf[1024*2];
  memset(stderr_buf, 0, sizeof(stderr_buf));
  if (setvbuf(stderr, stderr_buf, _IOFBF, sizeof(stderr_buf))) {
     fprintf(stderr, "failed to change the buffer of stderr");
  }
 #endif
  
  static char stdout_buf[1024*2];
  memset(stdout_buf, 0, sizeof(stdout_buf));
  if (setvbuf(stdout, stdout_buf, _IOFBF, sizeof(stdout_buf))) {
     fprintf(stderr, "failed to change the buffer of stdout");
  }
      
  while (i < 20000000)
  {
    i++;
    {
		static const char SAMPLE[] = "Christoph Gärtner";
		char buffer[] = "Christoph G&auml;rtner";
		assert(decode_html_entities_utf8(buffer, NULL) == sizeof SAMPLE - 1);
		assert(strcmp(buffer, SAMPLE) == 0);
	}

	{
		static const char SAMPLE[] = "test@example.org";
		static const char INPUT[] = "test&#x40;example.org";
		char buffer[sizeof INPUT];
		assert(decode_html_entities_utf8(buffer, INPUT) == sizeof SAMPLE - 1);
		assert(strcmp(buffer, SAMPLE) == 0);
	}

    printf("%d stdout %lld\n", i, compute);
    if (i % 10 == 0) fprintf(stderr, "stderr: %d\n", i);
  }
  
  printf("end of printer\n");
  fflush(stdout);

  return EXIT_SUCCESS;
}


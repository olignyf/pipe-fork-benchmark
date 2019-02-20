/*
	UNDER CONSTRUCTION: UNFINISHED
*/

#include "entities.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#undef NDEBUG
#include <assert.h>

int main(void)
{
   FILE *fp = NULL;
   int pid = fork();
   if (pid == 0)
   {
    // child
    
     fp = freopen("output.txt", "w", stdout);
     printf("Hello\n");
   }
   
	{
		static const char SAMPLE[] = "Christoph Gärtner";
		static char buffer[] = "Christoph G&auml;rtner";
		assert(decode_html_entities_utf8(buffer, NULL) == sizeof SAMPLE - 1);
		assert(strcmp(buffer, SAMPLE) == 0);
	}

	{
		static const char SAMPLE[] = "test@example.org";
		static const char INPUT[] = "test&#x40;example.org";
		static char buffer[sizeof INPUT];
		assert(decode_html_entities_utf8(buffer, INPUT) == sizeof SAMPLE - 1);
		assert(strcmp(buffer, SAMPLE) == 0);
	}

	fprintf(stdout, "All tests passed :-)\n");
    if (fp) fclose(fp);
	return EXIT_SUCCESS;
}


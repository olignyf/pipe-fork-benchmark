/*
	Copyright 2019 Francois Oligny-Lemieux
	https://github.com/olignyf

	Distributed under the Boost Software License, Version 1.0
*/

#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <termios.h>
#include "util.h"

#undef NDEBUG
#include <assert.h>


int main(void)
{
   int pipefd[2];
   pipefd[0] = 0;
   pipefd[1] = 0;
   pipe(pipefd);
   
   struct tms start_tms;
   struct tms end_tms;
   times(&start_tms);
   
   pid_t pid = fork();
   
   if (pid == 0)
   {
      // child
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {} // send stdout to the pipe
      while ((dup2(pipefd[1], STDERR_FILENO) == -1) && (errno == EINTR)) {} // send stderr to the pipe
      close(pipefd[0]);    // close unused read end
      close(pipefd[1]);    // close unused write end

      char *argv[5];
      argv[0] = "time";
      argv[1] = "-v";
      argv[2] = "./forkbun";
      argv[3] = NULL;
      execvp("/usr/bin/time", argv);
      printf("failed to execvp, errno %d\n", errno);
      exit(EXIT_FAILURE);
   }
   else if (pid == -1)
   {
   }
   else
   {
      printf("Benchmarker starting...\n");
      close(pipefd[1]);  // close the write end of the pipe in the parent
      FILE * dbgOut = fopen("benchmarker-stdout-stderr.txt", "wb");
      char strbuf[2][1024*8]; // must encompass the stderr (if any) & stdout buffers combined
      unsigned int strbuf_valid[2];
      char buffer[1024*100];
      unsigned int buffer_size = sizeof(buffer);
      unsigned int buffer_valid = 0;
      int which_strbuf_last = 1;
      struct tms time_report;
      size_t size_wanted = sizeof(time_report);
      memset(buffer, 0, sizeof(buffer));
      int64_t totalReadSize = 0;
      const int size_save = 1000;
      //ASSERT(sizeof(strbuf[1])>size_wanted);
      while (1)
      {
        size_t read_size_asked = sizeof(buffer)-1-buffer_valid;
        ssize_t count = read(pipefd[0], buffer+buffer_valid, read_size_asked);
        //printf("pipe read return %d, asked %d\n", (int)count, read_size_asked);
        if (count > 0)
        {
          buffer_valid += count;
          buffer[buffer_valid] = '\0';
          
          if (buffer_valid + size_save >= buffer_size) 
          {
            // shift entire buffer 1000 to the left, cropping initial part.
            // TODO print or save ?
            memmove(buffer, buffer+size_save, buffer_size-size_save);
            buffer_valid -= size_save;
          }
          
          //fwrite(buffer, count, 1, dbgLog);fflush(dbgLog);
          // copy last bytes in buffer
          which_strbuf_last = ++which_strbuf_last % 2;
          if (count >= size_wanted)
          {  memcpy(strbuf[which_strbuf_last], buffer+count-size_wanted, size_wanted);
             strbuf_valid[which_strbuf_last] = size_wanted;
          }
          else
          {  memcpy(strbuf[which_strbuf_last], buffer, count);
             strbuf_valid[which_strbuf_last] = size_wanted;
          }

          totalReadSize += count;
          //printf("child: %s\n", buffer);
        }
        else if (count == 0)
        {
          printf("end read child pipe\n", buffer);
          break;
        }
        else if (count == -1)
        {
          if (errno == EINTR)
          {   continue;
          }
          printf("error read child pipe\n", buffer);
          break;
        }
      }
      
      close(pipefd[0]);
      printf("totalReadSize: %lld\n", totalReadSize);
      
      // UNUSED, work in progress
      /*
      if (strbuf_valid[which_strbuf_last] >= size_wanted)
      {
         
         memcpy(&time_report, strbuf[which_strbuf_last]+strbuf_valid[which_strbuf_last]-size_wanted, size_wanted);
         //printf("copy struct\n");
      }*/
            
      int waitStatus = 0;
      waitpid(pid, &waitStatus, 0);
  
      /* call times for capturing end times */
      times(&end_tms);

      /* Calculate times, unfortunately return zero, but why??? */
      clock_t clock_cpu_time = end_tms.tms_cutime - start_tms.tms_cutime;
      clock_t clock_utime = end_tms.tms_utime - start_tms.tms_utime;
      clock_t clock_stime = end_tms.tms_stime - start_tms.tms_stime;
      clock_t clock_cstime = end_tms.tms_cstime - start_tms.tms_cstime;

      //Unfortunately printfs return zero, but why???
      printf("cpu time %jd\n\n",  (intmax_t)clock_cpu_time);
      printf("cpu Utime %jd\n\n", (intmax_t)clock_utime);
      printf("cpu Stime %jd\n\n", (intmax_t)clock_stime);
      printf("cpu CStime %jd\n\n", (intmax_t)clock_cstime);
    
      float user_time = -1.0;
      float wall_clock = -1.0;
      float cpu_time = -1.0;
      char temp[256] = "";
      temp[sizeof(temp)-1] = '\0';
      char * lookup = "Command being timed:";
      char * strRet = strstr(buffer, lookup);
      if (strRet) 
      {
         lookup = "User time (seconds): ";
         strRet = strstr(strRet, lookup);
         if (strRet)
         {
            strRet += strlen(lookup);
            sscanf(strRet, "%f", &user_time);
            printf("User time: %f\n", user_time);
         }
         
         lookup = "System time (seconds): ";
         strRet = strstr(strRet, lookup);
         if (strRet)
         {
            strRet += strlen(lookup);
            sscanf(strRet, "%f", &cpu_time);
            printf("CPU time: %f\n", cpu_time);
         }
         
         lookup = "Elapsed (wall clock)";
         strRet = strstr(strRet, lookup);
         if (strRet)
         {
            strRet += strlen(lookup);
            lookup = ": ";
            strRet = strstr(strRet, lookup);
            strRet += strlen(lookup);
            sscanf(strRet, "%s", temp);
            printf("Wall clock: %s\n", temp);
         }
      }
   }
   
   fprintf(stdout, "End of benchmarker\n");
   return EXIT_SUCCESS;
}


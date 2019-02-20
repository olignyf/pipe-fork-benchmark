/*
	Copyright 2019 Francois Oligny-Lemieux
	https://github.com/olignyf

	Distributed under the Boost Software License, Version 1.0
*/

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

#define METHOD_USE_SIGNAL_CHECK_FROM_KILL 0
#define METHOD_USE_WAITPID_NONBLOCK_CHECK 0


int main(void)
{
   int pipefd[2];
   pipefd[0] = 0;
   pipefd[1] = 0;
   pipe(pipefd);
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

      char *argv[3];
      argv[0] = "load-and-prints";
      argv[1] = NULL;
      argv[2] = NULL;
      execvp("./load-and-prints", argv);
      printf("failed to execvp, errno %d\n", errno);
      exit(EXIT_FAILURE);
   }
   else if (pid == -1)
   {
   }
   else
   {
      // parent
      int iRet;      
      char buffer[1024];
      buffer[sizeof(buffer)-1] = '\0';
  
      const char * logFilePath = "./forkbun1.txt";
      FILE * dbgLog = fopen(logFilePath, "wb");
      
      char szTime[128] = "";
      szTime[sizeof(szTime)-1] = '\0';
      time_t now = time(NULL);
      if (now != -1)
      {
         struct tm * timeinfo = localtime(&now);
         if (timeinfo)
         {
            strftime(szTime, sizeof(szTime)-1, "%d-%m-%Y %H:%M:%S", timeinfo);
         }
         else strcpy(szTime, "localtime error");
      }
      
      snprintf(buffer, sizeof(buffer)-1, "Task log starting (%s).\n", szTime);
      if (dbgLog)
      {
        fwrite(buffer, strlen(buffer), 1, dbgLog);
        fflush(dbgLog);
      }
      else
      {
         fprintf(stderr, "Failed to open task log (%s)\n", logFilePath);
      }
      
      close(pipefd[1]);  // close the write end of the pipe in the parent

      int waitStatus = 0;
      iRet = waitpid(pid, &waitStatus, WNOHANG);
      if (iRet != 0 && dbgLog)
      {
        snprintf(buffer, sizeof(buffer)-1, "Process was not found, stopping logs, waitStatus(%d).\n", waitStatus);
        fputs(buffer, stderr);
        fwrite(buffer, strlen(buffer), 1, dbgLog);
        fclose(dbgLog);
        dbgLog = NULL;
      }
 
      memset(buffer, 0, sizeof(buffer));
      int64_t fileSize;
      while (dbgLog)
      {
#if METHOD_USE_SIGNAL_CHECK_FROM_KILL == 1
        iRet = kill(pid, 0);
        if (iRet != 0)
        {
          printf("NEW PROCESS EXITED %d\n", iRet);
          break;
        }
#endif

#if METHOD_USE_WAITPID_NONBLOCK_CHECK == 1
        waitStatus = 0;
        iRet = waitpid(pid, &waitStatus, WNOHANG);
        if (iRet != 0)
        {
          printf("NEW PROCESS WAITPID non-zero return %d\n", iRet);
          break;
        }
#endif

        ssize_t count = read(pipefd[0], buffer, sizeof(buffer)-1);
        printf("pipe read return %d\n", (int)count);
        if (count > 0)
        {
          //fwrite(buffer, count, 1, dbgLog);fflush(dbgLog);
          //printf("child: %s\n", buffer);
             
          size_t written = fwrite(buffer, count, 1, dbgLog);
          if (written > 0)
          {
            fileSize += count;
          }
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
      
      if (dbgLog) fclose(dbgLog);
      close(pipefd[0]);
      waitStatus = 0;
      waitpid(pid, &waitStatus, 0);
   }
   
  fprintf(stdout, "End of pipetest\n");
  return EXIT_SUCCESS;
}

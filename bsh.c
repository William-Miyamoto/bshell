// Bash shell project; William Miyamoto CS444 November 2024 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//accept up to 16 command-line arguments
#define MAXARG 16

//allow up to 64 environment variables
#define MAXENV 64

//keep the last 500 commands in history
#define HISTSIZE 500

//accept up to 1024 bytes in one command
#define MAXLINE 1024

static char **parseCmd(char cmdLine[]) {
  char **cmdArg, *ptr;
  int i;

  //(MAXARG + 1) because the list must be terminated by a NULL ptr
  cmdArg = (char **) malloc(sizeof(char *) * (MAXARG + 1));
  if (cmdArg == NULL) {
    perror("parseCmd: cmdArg is NULL");
    exit(1);
  }
  for (i = 0; i <= MAXARG; i++) //note the equality
    cmdArg[i] = NULL;
  i = 0;
  ptr = strsep(&cmdLine, " ");
  while (ptr != NULL) {
    // (strlen(ptr) + 1)
    cmdArg[i] = (char *) malloc(sizeof(char) * (strlen(ptr) + 1));
    if (cmdArg[i] == NULL) {
      perror("parseCmd: cmdArg[i] is NULL");
      exit(1);
    }
    strcpy(cmdArg[ i++ ], ptr);
    if (i == MAXARG)
      break;
    ptr = strsep(&cmdLine, " ");
  }
  return(cmdArg);
}

int main(int argc, char *argv[], char *envp[]) {
  char cmdLine[MAXLINE], **cmdArg;
  int status, i, debug, cmdcount, ec;
  int hdex, pdex;
  pid_t pid;
  ec = 0;
  cmdcount = 0;
  char *envar[64][2]; // env variable array
  for (int i = 0; i < 64; i++) envar[i][0] = 0;
  for (int i = 0; i < 64; i++) envar[i][1] = 0;
  for (int i = 0; i < 64; i++) {
    if (envp[i] != NULL) {
      char* value = strdup(envp[i]);
      char* name = strsep(&value, "=");
      // envar[i][0] = malloc(strlen(name) + 1);
      // envar[i][1] = malloc(strlen(value) + 1);
      envar[i][0] = strdup(name);
      envar[i][1] = strdup(value);
      if (strcmp(envar[i][0], "PWD") == 0 ) pdex = i;
      if (strcmp(envar[i][0], "HOME") == 0 ) hdex = i;
      ec++;
    } else break;
  }
  
  char *cmds[HISTSIZE]; // cmd history array
  for (int i = 0; i < HISTSIZE; i++) {
    cmds[i] = malloc(MAXLINE);
    cmds[i][0] = '\0';
  }

  debug = 0;
  i = 1;
  while (i < argc) {
    if (! strcmp(argv[i], "-d") )
      debug = 1;
    i++;
  }
  while (( 1 )) {
    printf("bsh> ");                      //prompt
    fgets(cmdLine, MAXLINE, stdin);       //get a line from keyboard
    cmdLine[strlen(cmdLine) - 1] = '\0';  //strip '\n'
    cmdArg = parseCmd(cmdLine);
    if (debug) {
      i = 0;
      while (cmdArg[i] != NULL) {
        printf("\t%d (%s)\n", i, cmdArg[i]);
        i++;
      }
    }

   if (cmdcount < HISTSIZE) {
      for (int i = 0; i < MAXARG && cmdArg[i] != NULL; i++) {
        strcat(cmds[cmdcount], cmdArg[i]);
        if (cmdArg[i + 1] != NULL) strcat(cmds[cmdcount], " ");
    } 
    cmdcount++;
  } else {
    for (int i = 0; i < HISTSIZE - 1; i++) {
      strcpy(cmds[i], cmds[i + 1]);  // move everything up
    }
    cmds[HISTSIZE - 1][0] = '\0'; 
    for (int i = 0; i < MAXARG && cmdArg[i] != NULL; i++) {
      strcat(cmds[HISTSIZE - 1], cmdArg[i]);
      if (cmdArg[i + 1] != NULL) strcat(cmds[HISTSIZE - 1], " ");
    }
  } 

    //built-in command exit
    if (strcmp(cmdArg[0], "exit") == 0) {
      if (debug)
	      printf("exiting\n");
      break;
    }
    //built-in command env
    else if (strcmp(cmdArg[0], "env") == 0) {
      for (int i = 0; i < 64; i++) {
        if (envar[i][0] != 0) printf("%s=%s\n", envar[i][0], envar[i][1]);
      }
    }
    //built-in command setenv
    else if (strcmp(cmdArg[0], "setenv") == 0) {
      int exists = 0;
      for (int i = 0; i < 64; i++) {
        if (envar[i][0] != 0) {
          if ((strcmp(cmdArg[1], envar[i][0]) == 0)) { // compare to variable
           // search existing list for var
            if(envar[i][1]) free(envar[i][1]);
            envar[i][1] = strdup(cmdArg[2]);
            printf("Environment Variable %s set to %s\n", cmdArg[1], envar[i][1]);
            exists = 1;
            break;
          }
        } else break;
      }
      if (exists == 0) { // handle new environment variable setting
        printf("debug ec is %d\n", ec);
        if (ec < 64) {
          if (envar[ec+1][0]) free(envar[ec+1][0]);
          if (envar[ec+1][1]) free(envar[ec+1][1]);
          envar[ec + 1][0] = strdup(cmdArg[1]);
          envar[ec + 1][1] = strdup(cmdArg[2]);
          printf("Environment Variable %s set to %s\n", envar[ec + 1][0], envar[ec + 1][1]);
          if (strcmp(cmdArg[1], "PWD") == 0) pdex = ec + 1;
          if (strcmp(cmdArg[1], "HOME") == 0) pdex = ec + 1;
          ec++;
        } else printf("Error: maximum environment variables reached.");
      }
    }
    //built-in command unsetenv
    else if (strcmp(cmdArg[0], "unsetenv") == 0) {
      for (int i = 0; i < 64; i++) {
        if (envar[i][0] != 0) {
          if ((strcmp(cmdArg[1], envar[i][0]) == 0)) {
           // search existing list for var
            for (int j = i; j < 63; j++) { // shift remaining environment vars down
              envar[j][0] = envar[j + 1][0];
              envar[j][1] = envar[j + 1][1];
            }
            if (i < pdex) pdex--;
            if (i < hdex) hdex--;
            printf("Variable %s has been unset\n", cmdArg[1]);
            break;
          }
        }
      }
    }
    //built-in command cd
    else if (strcmp(cmdArg[0], "cd") == 0) {
      char* cwd = malloc(MAXLINE); // buffer for current working directory
      if (cmdArg[1] == NULL || strcmp(cmdArg[1], "~") == 0) { // default cd/home behavior
        if (chdir(envar[hdex][1]) != -1) {
          getcwd(cwd, MAXLINE);
          if (envar[pdex][1]) {
            free(envar[pdex][1]);
            envar[pdex][1] = NULL;
            }
          envar[pdex][1] = strdup(cwd);
        } else perror("chdir failed\n");
      } 
      else { // navigate to dir
        if (chdir(cmdArg[1]) != -1) {
          getcwd(cwd, MAXLINE);
          if (envar[pdex][1]) {
            free(envar[pdex][1]);
            envar[pdex][1] = NULL;
            }
          envar[pdex][1] = strdup(cwd);
        } else perror("chdir failed\n");
      }
      free(cwd);
    }
    //built-in command history
    else if (strcmp(cmdArg[0], "history") == 0) {
      for (int i = 0; i < cmdcount; i++) {
        printf("%s\n", cmds[i]);
      }
    }

    else {
      char *ptr, *ptr1, *pathtemp, *oindx;
      for (int i = 0; i < 64; i++) {
        if(envar[i][0] != NULL) {
          if(strcmp(envar[i][0], "PATH") == 0) {
            pathtemp = strdup(envar[i][1]);
            oindx = pathtemp; // save original pointer for freeing memory
            break;
          }
        }
      }
      while ((ptr = strsep(&pathtemp, ":")) != NULL) { // for each path...
        ptr1 = malloc(strlen(ptr) + strlen(cmdArg[0]) + 2); // string append handlers
        snprintf(ptr1, (strlen(ptr) + strlen(cmdArg[0]) + 2), "%s/%s", ptr, cmdArg[0]);  
        if (access(ptr1, X_OK) == 0) { // if it's a valid executable...
          pid = fork(); // fork process
          if (pid != 0) { // wait for child to complete
            waitpid(pid, &status, 0);
          } else if (pid != -1) { // child process runs
            // begin I/O handling
            int fid;
            for (int i = 0; i < MAXARG && cmdArg[i] != NULL; i++) { // for every valid argument
              if (strcmp(cmdArg[i], ">") == 0) { // stdout
                fid = open(cmdArg[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU); // open filestream, S_IRWXU is symbolic constant for rwe permissions
                close(1);
                dup(fid);
                close(fid);
                for (int j = i; cmdArg[j] != NULL; j++) { // remove the > and file args
                  cmdArg[j] = cmdArg[j + 2];
                }
                i--; // move back to original index
              }
              if (strcmp(cmdArg[i], "<") == 0) { // stdin 
                fid = open(cmdArg[i + 1], O_RDONLY); // open filestream
                close(0);
                dup(fid);
                close(fid);
                for (int j = i; cmdArg[j] != NULL; j++) { // remove the < and file args
                  cmdArg[j] = cmdArg[j + 2];
                }
                i--; // move back to original index
              }
            }
          // end I/O handling
            status = execv(ptr1, cmdArg);
          }
          free(ptr1); // free ptr1 if we're done
          break; // don't search any more paths
        }
        free(ptr1); // free ptr1 if we have to continue before re-mallocing it
      }
      free(oindx);
    }

    //clean up before running the next command
    i = 0;
    while (cmdArg[i] != NULL)
      free( cmdArg[i++] );
    free(cmdArg);
  }

  return 0;
}

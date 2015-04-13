#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

int IsPipeline = 0;

typedef struct {
    char **arguments;
    char *command;    
    char *infile;   
    char *outfile;  
} Full_Command;

int input, output;
char* inputFile;
char* outputFile;
char* token;
Full_Command commands[3];
Full_Command commands2[3];

void getCommand(char** args) {
  char* arguments1[MAX_TOKEN_COUNT];
  char* arguments2[MAX_TOKEN_COUNT];
  char* arguments3[MAX_TOKEN_COUNT];
  commands[0].command = NULL;
  commands[0].arguments = arguments1;
  commands[0].infile = NULL;
  commands[0].outfile = NULL;
  commands[1].command = NULL;
  commands[1].arguments = arguments2;
  commands[1].infile = NULL;
  commands[1].outfile = NULL;
  commands[2].command = NULL;
  commands[2].arguments = arguments3;
  commands[2].infile = NULL;
  commands[2].outfile = NULL;

  int currentCommand = 0;

  int i = 0;
  int argLocation = 0;
  int arg1 = 0;

  while(args[i] != NULL) {

    //new command for pipe
    if (strcmp(args[i], "|") == 0) {

      if (currentCommand == 0) {
        arg1 = argLocation;
        argLocation = 0;
      }

      currentCommand++;
      IsPipeline = 1;
    }

    //input
    else if (strcmp(args[i], "<") == 0){
      if (currentCommand == 0) {
        commands[0].infile = args[i+1];
      }
      else if (currentCommand == 1) {
        commands[1].infile = args[i+1];
      }
      
      i++;
    }

    //output
    else if (strcmp(args[i], ">") == 0){
      if (currentCommand == 0) {
        commands[0].outfile = args[i+1];
      }
      else if (currentCommand == 1) {
        commands[1].outfile = args[i+1];
      }
      i++;
    }

    //build args
    else {
      if (currentCommand == 0) {
        commands[0].arguments[argLocation] = args[i];
      }
      else if (currentCommand == 1) {
        commands[1].arguments[argLocation] = args[i];
      }
      argLocation++;
    }

    i++;
  }

  if (IsPipeline == 0) {
    commands[0].command = commands[0].arguments[0];
    commands[0].arguments[argLocation] = NULL;
  }
  else {
    commands[0].command = commands[0].arguments[0];
    commands[0].arguments[arg1] = NULL;
  }


  if (IsPipeline == 1){
    commands[1].command = commands[1].arguments[0];
    commands[1].arguments[argLocation] = NULL;
  }

}

void run(char **cmd) {
  int status; 

  switch (fork()) {
  case 0: /* child */
    execvp(cmd[0], cmd);
    perror(cmd[0]);   /* execvp failed */
    exit(1);

  default: /* parent */
    while (wait(&status) != -1) ; /* pick up dead children */
    break;

  case -1: /* error */
    perror("fork");
  }
  return;
}

void
runpipe(int pfd[])
{
  int pid;

  switch (pid = fork()) {

  case 0: /* child */
    dup2(pfd[0], 0);
    close(pfd[1]);
    execvp(commands[1].arguments[0], commands[1].arguments);

  default: /* parent */
    dup2(pfd[1], 1);
    close(pfd[0]);  
    execvp(commands[0].arguments[0], commands[0].arguments);

  case -1:
    perror("fork");
    exit(1);
  }
}



void runcmd(int fd, char **cmd, int type) {
  int status; 

  switch (fork()) {
  case 0: /* child */
    dup2(fd, type);  /* fd becomes the standard output */
    execvp(cmd[0], cmd);
    perror(cmd[0]);   /* execvp failed */
    exit(1);

  default: /* parent */
    while (wait(&status) != -1) ; /* pick up dead children */
    break;

  case -1: /* error */
    perror("fork");
  }
  return;
}

void runcmd2(int fd, int fd2, char **cmd, int type, int type2) {
  int status; 

  switch (fork()) {
  case 0: /* child */
    dup2(fd, type);
    dup2(fd2, type2); 
    execvp(cmd[0], cmd);
    perror(cmd[0]);   /* execvp failed */
    exit(1);

  default: /* parent */
    while (wait(&status) != -1) ; /* pick up dead children */
    break;

  case -1: /* error */
    perror("fork");
  }
  return;
}

void runcommand(char* command, char** args) {
  getCommand(args);

  //printf("%i\n", IsPipeline);

  //piped command
  if (IsPipeline > 0) {

    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
      runpipe(fd);
    }
    else {
      waitpid(pid, NULL, 0);
    }
  }

  else {
    if (commands[0].infile != NULL && commands[0].outfile != NULL) {
      output = open(commands[0].outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644);
      input = open(commands[0].infile, O_RDONLY, 0644);
      runcmd2(input, output, commands[0].arguments, 0, 1);
    }
    else if (commands[0].infile != NULL) {
      input = open(commands[0].infile, O_RDONLY, 0644);
      runcmd(input , commands[0].arguments, 0);
    }
    else if (commands[0].outfile != NULL) {
        output = open(commands[0].outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644);
        runcmd(output, commands[0].arguments, 1);
    }
    else {
        run(commands[0].arguments);
    }
  }
}

int main(){
    char line[MAX_LINE_LENGTH];
    printf("shell: ");
    while(fgets(line, MAX_LINE_LENGTH, stdin)) {
      line[strlen(line)-1] = '\0'; 
      char* command = NULL;
      char* arguments[MAX_TOKEN_COUNT];
      int argument_count = 0;

      token = strtok(line, " ");
      while(token) {   
          if(!command)
                command = token;     
          arguments[argument_count] = token;  
          argument_count++;                   
          token = strtok(NULL, " ");        
        }
      arguments[argument_count] = NULL;
        if(argument_count>0){
            if (strcmp(arguments[0], "exit") == 0)
                exit(0);
        runcommand(command, arguments);
        }
        printf("shell: "); 
    }
    return 0;
}

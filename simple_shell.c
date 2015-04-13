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


/*
void getCommand(char** args) {
  IsPipeline = 0;
  char* arguments1[MAX_TOKEN_COUNT];
  commands[0].command = NULL;
  commands[0].arguments = arguments1;
  commands[0].infile = NULL;
  commands[0].outfile = NULL;

  commands[1].command = NULL;
  commands[1].arguments = arguments1;
  commands[1].infile = NULL;
  commands[1].outfile = NULL;

  commands[2].command = NULL;
  commands[2].arguments = arguments1;
  commands[2].infile = NULL;
  commands[2].outfile = NULL;

  int currentCommand = 0;

  int i = 0;
  int argLocation = 0;

  int arg1 = 0;
  int arg2 = 0;
  int arg3 = 0;

  while(args[i] != NULL) {

    //new command for pipe
    if (strcmp(args[i], "|") == 0) {
      if (currentCommand == 0) {
        arg1 = argLocation;
      }
      else if (currentCommand == 1) {
        arg2 = argLocation;
      }

      argLocation = 0;

      IsPipeline = 1;
      currentCommand++;
      Full_Command newCmd;
      commands[currentCommand] = newCmd;
      commands[currentCommand].command = NULL;
      commands[currentCommand].arguments = arguments1;
      commands[currentCommand].infile = NULL;
      commands[currentCommand].outfile = NULL;
    }

    //input
    else if (strcmp(args[i], "<") == 0){
      commands[currentCommand].infile = args[i+1];
      i++;
    }

    //output
    else if (strcmp(args[i], ">") == 0){
      commands[currentCommand].outfile = args[i+1];
      i++;
    }

    //build args
    else {
      commands[currentCommand].arguments[argLocation] = args[i];
      argLocation++;
    }


    i++;
  }

  commands[0].command = commands[0].arguments[0];
  //if (arg1 == 0) {
    //commands[0].arguments[argLocation] = NULL;
  //}
  //else {
    //commands[0].arguments[arg1] = NULL;
  //}
  

  if (commands[1].arguments[0] != NULL) {
    commands[1].command = commands[1].arguments[0];
    commands[1].arguments[arg2] = NULL;
  }
    

  if (commands[2].arguments[0] != NULL) {
    commands[2].command = commands[2].arguments[0];
    commands[2].arguments[argLocation] = NULL;
  }
    

}*/

void getCommand(char** args) {
  char* arguments1[MAX_TOKEN_COUNT];
  commands[0].command = NULL;
  commands[0].arguments = arguments1;
  commands[0].infile = NULL;
  commands[0].outfile = NULL;

  //commands[1] = NULL;
  //commands[2] = NULL;

  int currentCommand = 0;

  int i = 0;
  int argLocation = 0;

  while(args[i] != NULL) {

    //new command for pipe
    if (strcmp(args[i], "|") == 0) {
      currentCommand++;
      Full_Command newCmd;
      commands[currentCommand] = newCmd;
      commands[currentCommand].command = NULL;
      commands[currentCommand].arguments = arguments1;
      commands[currentCommand].infile = NULL;
      commands[currentCommand].outfile = NULL;
    }

    //input
    else if (strcmp(args[i], "<") == 0){
      commands[currentCommand].infile = args[i+1];
      i++;
    }

    //output
    else if (strcmp(args[i], ">") == 0){
      commands[currentCommand].outfile = args[i+1];
      i++;
    }

    //build args
    else //if (commands[currentCommand].command != NULL){
      {
        commands[currentCommand].arguments[argLocation] = args[i];
      argLocation++;
    }

    //else {
    //  commands[currentCommand].command = args[i];
   // }

    i++;
  }

  commands[0].command = commands[0].arguments[0];
  commands[0].arguments[argLocation] = NULL;
  //if (commands[1].arguments != NULL)
  //  commands[1].command = commands[1].arguments[1];
  //if (commands[2].arguments != NULL)
  //  commands[2].command = commands[2].arguments[2];

}

void runpipe(int pfd[]) {
  pid_t pid = fork();

  if (pid == 0) {
      dup2(pfd[0], 0);
      close(pfd[1]);
      execvp(commands[1].command, commands[1].arguments);
  }

  else {
      dup2(pfd[1], 1);
      close(pfd[0]);
      execvp(commands[0].command, commands[0].arguments);
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
    dup2(fd2, type2);  /* fd becomes the standard output */
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


  //piped command
  if (IsPipeline == 1) {

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
    /*
    pid_t pid = fork();

    //child proccess
    if (pid == 0) {
      if (commands[0].infile != NULL) {
        input = open(commands[0].infile, O_RDONLY);
        dup2(input, 0);
      }
      if (commands[0].outfile != NULL) {
        printf("%s\n", commands[0].outfile);
        output = open(commands[0].outfile, O_WRONLY | O_TRUNC | O_CREAT | S_IRUSR | S_IWUSR);
        dup2(output, 1);
      }
      execvp(commands[0].command, commands[0].arguments);
    }

    //parent process
    else {
      waitpid(pid, NULL, 0);
    }*/
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
      while(token) {                      //while there is at least one argument
          if(!command)
                command = token;            //set command to first arg
          arguments[argument_count] = token;  //set first arg to command too?
          argument_count++;                   //increment to next arg
          token = strtok(NULL, " ");          //set token to next arg
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

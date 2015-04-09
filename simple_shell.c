#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>


#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid(). 

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
**/

int input, output;
char* inputFile;
char* outputFile;
char* token;

void runcommand(char* command, char** args) {
  pid_t pid = fork();
  if(pid) { // parent
    	waitpid(pid, NULL, 0);
  } else { // child
      int i;
      while(args[i] != NULL){
      
          if (strcmp(args[i], "<") == 0){                   //if redirect input
              inputFile = args[i+1];
              input = open(inputFile, O_RDONLY);
              dup2(input, 0);
              args[i] = NULL;
              args[i+1] = NULL;
          }
          
          if (strcmp(args[i], ">") == 0){                  //if redirect output
              outputFile = args[i+1];
              output = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT | S_IRUSR | S_IWUSR);
              dup2(output, 1);
              args[i] = NULL;
              args[i+1] = NULL;
          }
          execvp(command, args);
      }
    	
  }
}

int main(){
    

    char line[MAX_LINE_LENGTH];
    printf("shell: ");
    while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    	// Build the command and arguments, using execv conventions.
    	line[strlen(line)-1] = '\0'; // get rid of the new line
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
   char *commandText;
   char **commandArguments;
   struct Command* nextCommand;
   int piped;
   int background;
   int argc;
} Command;

char * addStringEnding(char *unendedString);

void printCommand(Command *command){
  int i;
  printf("command: %s\n", command->commandText);
  printf("number of args: %d\n", command->argc);
  for(i=0; i<=command->argc; i++){
    printf("argument: %s\n", command->commandArguments[i]);
  }
}

int main(void)
{
    Command *currentCommand = malloc(sizeof(Command));
    char commandBuffer[300];
    int maxArgs = 10;
    char *tempArgs[maxArgs];
    int i;

    while(1){
      memset(commandBuffer, 0, sizeof(commandBuffer));
      printf(">");

      if (fgets(commandBuffer, 300, stdin) != NULL) {

        char * pos;
        if ((pos=strchr(commandBuffer, '\n')) != NULL){
          *pos = '\0';
        }

        //parsing args for command
        tempArgs[0] = strtok(commandBuffer, " ");

        i = 1;
        while (i < maxArgs){
          tempArgs[i] = strtok(NULL, " ");
          if (tempArgs[i] == NULL){
            break;
          }
          i++;
        }

        currentCommand->commandArguments = (char **)malloc((i+1) * sizeof(char*));
        currentCommand->commandArguments[i] = (char *) 0;
        memcpy(currentCommand->commandArguments, &tempArgs[0], i*sizeof(char*));
        currentCommand->argc = i;
        currentCommand->commandText = currentCommand->commandArguments[0];

        printCommand(currentCommand);

        fflush(stdout);

        //execution of command
        int status;
        int pid = fork();
        if (pid == 0) {
          //child process
          execvp (currentCommand->commandText, currentCommand->commandArguments);
          //execvp ("ls\n", currentCommand->commandArguments);
          perror("execvp failure");
          return 1;
        }
        else {
          //parent process
          waitpid(pid, &status, 0);
        }
      }
    }
    return 0;
}

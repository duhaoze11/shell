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
      printf(">");

      if (fgets(commandBuffer, 300, stdin) != NULL) {

        //parsing args for command
        tempArgs[0] = strtok(commandBuffer, " ");
        printf("%s\n",tempArgs[0]);

        i = 1;
        while (i < maxArgs){
          printf("number of args %d\n", i);
          tempArgs[i] = strtok(NULL, " ");
          if (tempArgs[i] == NULL){
            break;
          }
          i++;
        }

        currentCommand->commandArguments = (char **)malloc((i+1) * sizeof(char*));
        currentCommand->commandArguments[i] = '\0';
        memcpy(currentCommand->commandArguments, &tempArgs[0], i*sizeof(char*));
        currentCommand->argc = i;
        currentCommand->commandText = currentCommand->commandArguments[0];

        printCommand(currentCommand);

        fflush(stdout);

        //execution of command
        int pid = fork();
        if (pid == 0) {
          execvp (currentCommand->commandText, currentCommand->commandArguments);
          perror("execvp failure");
          return 1;
        }
        else {
          // TODO: wait for child
        }
      }
    }
    return 0;
}

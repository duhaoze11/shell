#include "meliShell.h"

void printCommand(Command *command){
  int i;
  printf("command: %s ", command->commandText);
  printf("number of args: %d ", command->argc);
  for(i=0; i<=command->argc; i++){
    printf("argument: %s ", command->commandArguments[i]);
  }
  printf("\n");
}

int main(void)
{
    Command *currentCommand;
    char commandBuffer[300];
    int background;

    while(1){
      background = 0;
      memset(commandBuffer, 0, sizeof(commandBuffer));
      printf(">");

      if (fgets(commandBuffer, 300, stdin) != NULL) {

        //Strip the newline character
        char * pos;
        if ((pos=strchr(commandBuffer, '\n')) != NULL){
          *pos = '\0';
        }

        //Find the background execution terminator
        //TODO: check if it's in the end
        if ((pos=strchr(commandBuffer, '&')) != NULL){
          background = 1;
          *pos = '\0';
        }

        //parsing commands
        char *parsePtr;
        Command *newCommand;
        char *commandPart = strtok_r(commandBuffer, "|", &parsePtr);
        currentCommand = parseCommand(commandPart);

        while ((commandPart = strtok_r(NULL, "|", &parsePtr)) != NULL){
          newCommand = parseCommand(commandPart);
          newCommand->nextCommand = currentCommand;
          currentCommand = newCommand;
        }

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
          if (!background) {
            waitpid(pid, &status, 0);
          }
        }
      }
    }
    return 0;
}

Command * parseCommand(char * commandBuffer){
  //TODO: Parse strings as parameters
  int maxArgs = 10;
  char *tempArgs[maxArgs];
  Command *newCommand = malloc(sizeof(Command));
  tempArgs[0] = strtok(commandBuffer, " ");

  int i = 1;
  while (i < maxArgs){
    tempArgs[i] = strtok(NULL, " ");
    if (tempArgs[i] == NULL){
      break;
    }
    i++;
  }

  newCommand->commandArguments = (char **)malloc((i+1) * sizeof(char*));
  newCommand->commandArguments[i] = (char *) 0;
  memcpy(newCommand->commandArguments, &tempArgs[0], i*sizeof(char*));
  newCommand->argc = i;
  newCommand->commandText = tempArgs[0];

  printCommand(newCommand);

  return newCommand;
}

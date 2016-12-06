#include "meliShell.h"

void printCommand(Command *command){
  int i;
  printf("command: %s ", command->commandText);
  printf("number of args: %d ", command->argc);
  for(i=0; i<=command->argc; i++){
    printf("argument: %s ", command->commandArguments[i]);
  }
  if (command->nextCommand){
    printf("has next command");
  } else {
    printf("no next command");
  }
  printf("\n");
}

int main(void)
{
    int bufferSize = 300;
    Command *currentCommand;
    int history;
    char commandBuffer[bufferSize];
    int background, pipes;
    FILE *historyFile;

    history = initializeHistory();

    while(1){
      background = 0; pipes = 0;
      memset(commandBuffer, 0, bufferSize);
      printf("> ");

      if (fgets(commandBuffer, 300, stdin) != NULL) {

        writeToHistory(&history, commandBuffer);

        //check for the history commandText
        if (strcmp (commandBuffer,"history\n") == 0){
          strcpy (commandBuffer,"cat .myhistory");
        }

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
          pipes++;
          newCommand = parseCommand(commandPart);
          newCommand->nextCommand = currentCommand;
          currentCommand = newCommand;
        }

        fflush(stdout);

        //execution of commands
        int status;
        int pid = fork();
        if (pid == -1){
          perror("Fork failed D:");
          return 2;
        }
        else if (pid == 0) {
          //child process
          if (pipes > 0){
              executePipedCommands(currentCommand);
          }
          if (pipes == 0){
            //execution of unique command
            execvp (currentCommand->commandText, currentCommand->commandArguments);
            perror("execvp failure");
            return 1;
          }
        }
        else {
          //parent process, waits if not background
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
  newCommand->nextCommand = NULL;

  return newCommand;
}

int executePipedCommands(Command *currentCommand){
  int i = 0;
  //Setting up pipes
  // writeToHistory(&i, "Setting up pipes for command\n");
  // writeToHistory(&i, currentCommand->commandText);
  // writeToHistory(&i, "\n");
  int fdes[2];
  if (pipe(fdes) == -1){
      perror("Pipes failed D:");
      return 1;
  }

  int pid = fork();
  if (pid == -1){
    perror("Fork failed D:");
    return 1;
  }
  else if (pid == 0){
    //child process, the one in the left of the pipe
    currentCommand = currentCommand->nextCommand;
    // writeToHistory(&i, "I am a child process preparing to write, I'll execute command:\n");
    // writeToHistory(&i, currentCommand->commandText);
    // writeToHistory(&i, "\n");

    dup2(fdes[WRITE], fileno(stdout));
    close(fdes[READ]);
    close(fdes[WRITE]);
    if(currentCommand->nextCommand != NULL){
      // writeToHistory(&i, "But there's more, so I'll fork more :)\n");
      // writeToHistory(&i, "\n");
      executePipedCommands(currentCommand);
    }
    execvp (currentCommand->commandText, currentCommand->commandArguments);
    return 1;

  }
  else {
    //parent process, the one in the right of the pipe
    dup2(fdes[READ], fileno(stdin));
    close(fdes[READ]);
    close(fdes[WRITE]);
    // writeToHistory(&i, "Parent process executing command now!:\n");
    // writeToHistory(&i, currentCommand->commandText);
    // writeToHistory(&i, "\n");
    execvp (currentCommand->commandText, currentCommand->commandArguments);
    perror("execvp failure in parent");
    return 1;
  }
}

int initializeHistory() {
    int historyBufferSize = 350;
    char historyBuffer[350];
    int history = 1;

    memset(historyBuffer, 0, historyBufferSize);
    FILE *historyFile = fopen(".myhistory", "a+");
    rewind(historyFile);
    while(fgets(historyBuffer, historyBufferSize, historyFile)) {
    }
    
    if (sscanf(historyBuffer, "%d *", &history) == EOF){
      history = 1;
    } else {
      history++;
    }
    
    fclose(historyFile);

    return history;
}

void writeToHistory(int *history, char *commandLine){
  FILE *historyFile = fopen(".myhistory", "a+");
  fprintf(historyFile, "%d %s",*(history), commandLine);
  *(history) = *(history)+1;
  fclose(historyFile);
}

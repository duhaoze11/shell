#include "meliShell.h"

int main(void)
{
    int bufferSize = 300;
    Command *currentCommand;
    int history;
    char commandBuffer[bufferSize];
    char *commandPointer, *inputFile, *outputFile;
    int background;

    history = initializeHistory();

    while(1){
      background = 0; commandPointer = NULL; inputFile = NULL; outputFile = NULL;
      memset(commandBuffer, 0, bufferSize);
      printf("> ");

      if (fgets(commandBuffer, 300, stdin) != NULL) {

        if (commandBuffer[0] == '!'){
          commandBuffer[0] = '0';
          if (!executeHistory(commandBuffer, bufferSize)){
            continue;
          }
        }

        writeToHistory(&history, commandBuffer);

        //Strip the newline character
        char * pos;
        if ((pos = strchr(commandBuffer, '\n')) != NULL){
          *pos = 0;
        }

        //Find the background execution terminator
        //TODO: check if it's in the end
        if ((pos = strchr(commandBuffer, '&')) != NULL){
          background = 1;
          *pos = 0;
        }

        char *redirectionPtr;
        if ((pos = strchr(commandBuffer, '<')) != NULL){
          inputFile = trim(strtok_r(commandBuffer, "<", &redirectionPtr));
        } else {
          redirectionPtr = commandBuffer;
        }
        commandPointer = strtok_r(NULL, ">", &redirectionPtr);
        outputFile = trim(strtok_r(NULL, ">", &redirectionPtr));

        //parsing commands
        char *parsePtr;
        Command *newCommand;
        char *commandPart = strtok_r(commandPointer, "|", &parsePtr);
        currentCommand = parseCommand(commandPart);
        if(inputFile){
          currentCommand->inputFile = inputFile;
        }

        while ((commandPart = strtok_r(NULL, "|", &parsePtr)) != NULL){
          newCommand = parseCommand(commandPart);
          newCommand->nextCommand = currentCommand;
          currentCommand = newCommand;
        }

        if(outputFile){
          currentCommand->outputFile = outputFile;
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
          if (!executePipedCommands(currentCommand)){
            continue;
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

  if (strstr(tempArgs[0], "history")){
    newCommand->commandArguments = (char **)malloc(3 * sizeof(char*));
    newCommand->commandArguments[0] = "cat";
    newCommand->commandArguments[1] = ".myhistory";
    newCommand->commandArguments[2] = (char *) 0;
    newCommand->argc = 3;
  } else {
    newCommand->commandArguments = (char **)malloc((i+1) * sizeof(char*));
    newCommand->commandArguments[i] = (char *) 0;
    memcpy(newCommand->commandArguments, &tempArgs[0], i*sizeof(char*));
    newCommand->argc = i;
  }

  newCommand->commandText = newCommand->commandArguments[0];
  newCommand->nextCommand = NULL; newCommand->inputFile = NULL; newCommand->outputFile = NULL;

  return newCommand;
}

int executePipedCommands(Command *currentCommand){
  int i = 0;
  int oldOut = dup(WRITE);

  //Setting up pipes
  // writeToHistory(&i, "Setting up pipes for command\n");
  // writeToHistory(&i, currentCommand->commandText);
  // writeToHistory(&i, "\n");
  int fdes[2];
  if (pipe(fdes) == -1){
      perror("Pipes failed D:");
      exit(0);
  }

  int pid = fork();
  if (pid == -1){
    perror("Fork failed D:");
    exit(0);
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
    else if(currentCommand->inputFile){
      //file redirection 
      int fin = open(currentCommand->inputFile, O_RDWR | O_CREAT);
      dup2(fin, fileno(stdin));
      close(fin);
    }
    execvp (currentCommand->commandText, currentCommand->commandArguments);
    dup2(oldOut, WRITE);
    close(oldOut);
    perror("execvp failure");
    exit(0);

  }
  else {
    //Parent process, the one in the right of the pipe
    dup2(fdes[READ], fileno(stdin));
    close(fdes[READ]);
    close(fdes[WRITE]);
    if(currentCommand->outputFile){
      //file redirection 
      int fout = open(currentCommand->outputFile, O_RDWR | O_TRUNC | O_CREAT);
      dup2(fout, fileno(stdout));
      close(fout);
    }

    // writeToHistory(&i, "Parent process executing command now!:\n");
    // writeToHistory(&i, currentCommand->commandText);
    // writeToHistory(&i, "\n");
    execvp (currentCommand->commandText, currentCommand->commandArguments);
    dup2(oldOut, WRITE);
    close(oldOut);
    perror("execvp failure");
    exit(0);
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

int executeHistory(char * commandBuffer, int bufferSize){
  int historyNumber = 0;
  sscanf(commandBuffer, "%d", &historyNumber);
  if (historyNumber == 0){
    printf("Invalid history command\n");
    return 0;
  }
  char historyCommand[bufferSize];
  sprintf(historyCommand, "cat .myhistory | grep -m 1 %d", historyNumber);
  FILE *fin = popen(historyCommand, "r");
  memset(historyCommand, 0, bufferSize);
  read(fileno(fin), historyCommand, bufferSize);

  char * pos;
  pos = strchr(historyCommand, ' ');
  (*pos)++;
  strcpy(commandBuffer, pos+1);
  return 1;
}

void printCommand(Command *command){
  int i;
  printf("command: %s ", command->commandText);
  printf("number of args: %d ", command->argc);
  for(i=0; i<=command->argc; i++){
    printf("argument: %s ", command->commandArguments[i]);
  }
  if (command->nextCommand){
    printf("has next command");
  } 
  if (command->inputFile){
    printf("input file: %s", command->inputFile);
  } 
  if (command->outputFile){
    printf("output file: %s", command->outputFile);
  } 
  printf("\n");
}

char * trim(char *str){
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if(str == NULL) { return NULL; }
    if(str[0] == '\0') { return str; }

    len = strlen(str);
    endp = str + len;
    while(isspace((unsigned char) *frontp)) { ++frontp; }
    if(endp != frontp){
      while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if(str + len - 1 != endp)
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';
    endp = str;
    if(frontp != str){
      while(*frontp) { *endp++ = *frontp++; }
      *endp = '\0';
    }
    return str;
}

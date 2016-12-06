#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct commandStruct{
   char *commandText;
   char **commandArguments;
   struct commandStruct* nextCommand;
   int argc;
} Command;

Command * parseCommand(char * commandBuffer);
int executePipedCommands(Command *currentCommand);
int initializeHistory();
void writeToHistory(int *history, char *commandLine);
int executeHistory(char * commandBuffer, int bufferSize);

enum {READ, WRITE};
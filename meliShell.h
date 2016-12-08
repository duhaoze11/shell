#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>

typedef struct commandStruct{
   char *commandText;
   char **commandArguments;
   struct commandStruct* nextCommand;
   char *inputFile;
   char *outputFile;
   int argc;
} Command;

Command * parseCommand(char * commandBuffer);
int executePipedCommands(Command *currentCommand);
int initializeHistory();
void writeToHistory(int *history, char *commandLine);
int executeHistory(char * commandBuffer, int bufferSize);
char * replace_str(char *str, char *orig, char *rep);
void printCommand(Command *command);
char * trim(char *str);

enum {READ, WRITE};

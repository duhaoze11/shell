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

enum {READ, WRITE};
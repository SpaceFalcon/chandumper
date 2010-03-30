#include "randstring.h"

#include <stdlib.h>
#include <string.h>

char *getRandomString(int length)
{
    char *finalString = malloc(length) + 1;
    memset(finalString, 0, length + 1);
    char possibilities[] = "abcdefghijklmnopqrstuvwxyz1234567890:;[]{}";
    int i = 0;
    for(i = 0; i < length; i++)
    {
        finalString[i] = possibilities[rand() % sizeof(possibilities)];
    }
    return finalString;
}

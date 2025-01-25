#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>

typedef unsigned char byte;

#define MAX_DATA_LEN 65536
#define MAX_FILENAME_LEN 512
#define MAX_PLAYLIST_LEN 1000000
#define MAX_PLAYLIST_ROWLEN 600

#define Print2Console

int m_round(double d);

static char bufferMessage[100];

void print2Console(char* message);

#endif
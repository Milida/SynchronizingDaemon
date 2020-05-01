#ifndef filelib
#define filelib
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <sys/sendfile.h>
#include <utime.h>
#include <signal.h>
#include <stdbool.h>
#define BUFF_SIZE 64

typedef struct al {
    char *name;
} allocation;

void handler(int signum);
int isDirectoryExists(const char *path);
int isFileExists(const char *path);
mode_t read_chmod(char *source);
time_t read_time(char *source);
off_t read_size(char *source);
void copy_File(char *sourceFile, char *destinationFile);
void copyFile(char *sourceFile, char *destinationFile);
void deleteFile(char *destinationFile, char *sourceFile);

#endif //filelib
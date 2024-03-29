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
#define BUFF_SIZE 100

typedef struct al {
    char *name;
} allocation;

void handler(int signum);
int dirExists(const char *path);
int fileExists(const char *path);
mode_t read_chmod(char *source);
time_t read_time(char *source);
off_t read_size(char *source);
void copyFile(char *sourceFile, char *destinationFile, int fileSize);
void copyDir(char *source, char *destination, int fileSize);
void demonCp(char *source, char *destination, bool recursive, int fileSize);
void deleteFromDir(char *source, char *destination, bool recursive);
char *catDir(char *ptr,char *path, char *name);

#endif //filelib
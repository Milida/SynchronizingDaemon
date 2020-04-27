#include <stdio.h>
#include <sys/stat.h>

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    stat(path, &stats);

    return (S_ISDIR(stats.st_mode));
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);

    return S_ISREG(stats.st_mode);
}
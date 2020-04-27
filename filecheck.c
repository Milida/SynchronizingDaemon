#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "filecheck.h"

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    if(stat(path, &stats) != -1){
        return (IS_ISDIR(stats.st_mode))
    }
    return 0;
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);

    return S_ISREG(stats.st_mode);
}
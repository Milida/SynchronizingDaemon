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
#include "filelib.h"
#include <utime.h>
#include <signal.h>
#define BUFF_SIZE 64

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    if(stat(path, &stats) != -1)
        return S_ISDIR(stats.st_mode);
    return 0;
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);
    if(stat(path,&stats) != -1){
        return S_ISREG(stats.st_mode);
    }
    return 0;
}
mode_t read_chmod(char *source){
    struct stat mode;
    if(stat(source, &mode) != -1)
        return mode.st_mode;
    else {
        syslog(LOG_ERR,"Couldn't take a file chmod");
        exit(EXIT_FAILURE);
    }
}

time_t read_time(char *source){
    struct stat time;
    if(stat(source,&time) != -1)
        return time.st_mtime;
    else{
        syslog(LOG_ERR,"Couldn't take a modification time");
        exit(EXIT_FAILURE);
    }
}

off_t read_size(char *source){
    struct stat size;
    if(stat(source, &size) != -1)
        return size.st_size;
    else{
        syslog(LOG_ERR,"Couldn't take a file size");
        exit(EXIT_FAILURE);
    }
}

void copy_File(char *sourceFile, char *destinationFile) {
    char bufor[4096];
    int readSource, writeDes;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if(destination < 0 && errno == EEXIST){
        //("Plik %s już istnieje\n", sourceFile);
        if(read_time(sourceFile) == read_time(destinationFile)) return;
    }
    destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
    if ((source < 0 || destination < 0) && errno != EEXIST) {
        printf("Couldn't open the file");
        syslog(LOG_ERR,"Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    while ((readSource = read(source, bufor, sizeof(bufor))) > 0){
        writeDes = write(destination, bufor, (ssize_t) readSource);
    }
    syslog(LOG_INFO,"File copied successfully: %s", destinationFile);
   // printf("Copying %s file\n",sourceFile);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){
        syslog(LOG_ERR,"Couldn't change the chmod");
        exit(EXIT_FAILURE);
    }
    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(utime(destinationFile, &source_time)){
        syslog(LOG_ERR,"Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void copyFile(char *sourceFile, char *destinationFile) {
    struct stat stbuf;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if(destination < 0 && errno == EEXIST){
        //printf("Plik %s już istnieje\n", sourceFile);
        if(read_time(sourceFile) == read_time(destinationFile)) return;
    }
    destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
    if ((source < 0 || destination < 0) && errno != EEXIST) {
       // printf("Couldn't open the file");
        syslog(LOG_ERR,"Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    //printf("Copying %s file\n",sourceFile);
    if(fstat(source, &stbuf)==-1){
       // printf("Fstat errno %d\n",errno);
        syslog(LOG_ERR,"Couldn't copy the file");
    }
    if(sendfile(destination, source, 0, stbuf.st_size)==-1){
        //printf("%d\n",errno);
       // printf("Couldn't copy the file\n");
        syslog(LOG_ERR,"Couldn't copy the file");
    }
    syslog(LOG_INFO,"File copied successfully: %s", destinationFile);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){
        syslog(LOG_ERR,"Couldn't change the chmod");
        exit(EXIT_FAILURE);
    }

    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(utime(destinationFile, &source_time)){
        syslog(LOG_ERR,"Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void deleteFile(char *destinationFile, char *sourceFile) {
    int destination = open(destinationFile, O_RDONLY);
    int source = open(sourceFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if (source < 0 && errno == EEXIST) {
        printf("Plik %s już istnieje  %d\n", sourceFile, errno);
        return;
    } else if (remove(destinationFile) == 0){
        printf("\nDeleted successfully %s\n", destinationFile);
        syslog(LOG_INFO,"File deleted successfully");
    }
    remove(sourceFile);

    close(source);
    close(destination);
}
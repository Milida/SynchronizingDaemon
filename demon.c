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

typedef struct example {
    char *name;
} MyExample;

/*
** http://www.unixguide.net/unix/programming/2.5.shtml
** About locking mechanism...
*/

mode_t read_chmod(char *source){
    struct stat mode;
    if(stat(source, &mode) != -1){
        return mode.st_mode;
    }
    else exit(EXIT_FAILURE);
}

time_t read_time(char *source){
    struct stat time;
    if(stat(source,&time) != -1){
        return time.st_mtime;
    }
    else exit(EXIT_FAILURE);
}

off_t read_size(char *source){
    struct stat size;
    if(stat(source, &size) != -1){
        return size.st_size;
    }
    else exit(EXIT_FAILURE);
}

void copy_File(char *sourceFile, char *destinationFile) {
    char bufor[4096];
    int readSource, writeDes;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if(destination < 0 && errno == EEXIST){
        printf("Plik %s już istnieje  %d\n", sourceFile, errno);
        if(read_time(sourceFile) == read_time(destinationFile)) return;
    }
    destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
    if ((source < 0 || destination < 0) && errno != EEXIST) {
        printf("Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    while ((readSource = read(source, bufor, sizeof(bufor))) > 0){
        writeDes = write(destination, bufor, (ssize_t) readSource);
    }
    printf("Copying %s file\n",sourceFile);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){
        exit(EXIT_FAILURE);
    }
}


void copyFile(char *sourceFile, char *destinationFile) {
    struct stat stbuf;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if(destination < 0 && errno == EEXIST){
        printf("Plik %s już istnieje  %d\n", sourceFile, errno);
        if(read_time(sourceFile) == read_time(destinationFile)) return;
    }
    destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
    if ((source < 0 || destination < 0) && errno != EEXIST) {
        printf("Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    printf("Copying %s file\n",sourceFile);
    if(fstat(source, &stbuf)==-1){
        printf("Fstat errno %d\n",errno);
    }
    if(sendfile(destination, source, 0, stbuf.st_size)==-1){
        printf("%d\n",errno);
        printf("Couldn't copy the file\n");
    }

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){
        exit(EXIT_FAILURE);
    }

    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(utime(destinationFile, &source_time)){
        exit(EXIT_FAILURE);
    }
}

void deleteFile(char *destinationFile, char *sourceFile) {
    int destination = open(destinationFile, O_RDONLY);
    int source = open(sourceFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if (source < 0 && errno == EEXIST) {
        printf("Plik %s już istnieje  %d\n", sourceFile, errno);
        return;
    } else if (remove(destinationFile) == 0)
        printf("\nDeleted successfully %s\n", destinationFile);
    remove(sourceFile);

    close(source);
    close(destination);
}

void handler(int signum){
    puts("Handler");//#FIXME tutaj syslog
}

int main(int argc, char *argv[]){
    unsigned int sleepTime = 300;
    if(argc <= 2){
        printf("Too few arguments\n");
        exit(EXIT_FAILURE);
    }
    if(argc > 3){
        switch(argv[3]){
            case "-r":
                puts("opcja -r");
                break;
            case "s":
                puts("sleepTime");
                break;
            case "d":
                puts("rozmiar");
                break;
            case "-rs":
                puts("-r i sleepTime");
                brek;
            case "-rd":
                puts("-r i rozmiar");
                break;
            case "sd":
                puts("sleepTime i rozmiar");
                break;
        }
        printf("To many arguments\n");
        exit(EXIT_FAILURE);
    }
    char *source = argv[1];
    char *destination = argv[2];
    if (!isDirectoryExists(source)){
        exit(EXIT_FAILURE);
    }
    if(!isDirectoryExists(destination)) {
        exit(EXIT_FAILURE);
    }
    /* Our process ID and Session ID */
    pid_t pid, sid;
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
    we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);
    /* Open any logs here */
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Daemon-specific initialization goes here */
    signal(SIGUSR1, handler);
    /* The Big Loop */
    while (1) {
        DIR *sourceDir; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
        struct dirent *ep;
        sourceDir = opendir (source);
        MyExample *name = (MyExample*) malloc( sizeof(MyExample) );
        name->name = (char*) malloc(BUFF_SIZE);
        MyExample *des = (MyExample*) malloc( sizeof(MyExample) );
        des->name = (char*) malloc(BUFF_SIZE);
        //strcpy(name, source);
        //strcat(name,"/");
        if (sourceDir != NULL){
            while (ep = readdir (sourceDir)){
                strcpy(name->name, source);
                strcat(name->name,"/");
                if(isFileExists(strcat(name->name,ep->d_name))){
                    strcpy(des->name,destination);
                    strcat(des->name,"/");
                    if(read_size(name->name) < 15){
                        copy_File(name->name,strcat(des->name, ep->d_name));
                    }
                    else copyFile(name->name,strcat(des->name, ep->d_name));
                }
            }
            (void) closedir (sourceDir);
        }
        else{
            perror ("Couldn't open the directory");
        }

        DIR *desDir;
        struct dirent *epp;
        desDir = opendir (destination);
        MyExample *na =(MyExample *)malloc(sizeof(MyExample));
        na->name = (char *)malloc(BUFF_SIZE);
        MyExample *desti =(MyExample *)malloc(sizeof(MyExample));
        desti->name = (char *)malloc(BUFF_SIZE);

        if (desDir != NULL){
            while (epp = readdir (desDir)){
                strcpy(na->name, destination);
                strcat(na->name,"/");
                if(isFileExists(strcat(na->name,epp->d_name))){
                    strcpy(desti->name, source);
                    strcat(desti->name,"/");
                    deleteFile(na->name, strcat(desti->name, epp->d_name));
                }
            }
            (void) closedir (desDir);
        }
        else{
            perror ("Couldn't open the directory");
        }
        free(name);
        free(des);
        free(na);
        free(desti);
        sleep(sleepTime);
        /* Do some task here ... */
    }
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    exit(EXIT_SUCCESS);
}
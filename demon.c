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
#include <fcntl.h>
#include "filecheck.h"
#include "filelist.h"
#include <utime.h>

/*
** http://www.unixguide.net/unix/programming/2.5.shtml
** About locking mechanism...
*/
/*void copy_File(char *sourceFile, char *destinationFile) {
    char bufor[4096];
    int readSource, writeDes;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (source < 0 || destination < 0){
        printf("Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    while ((readSource = read(source, bufor, sizeof(bufor))) > 0){
        writeDes = write(destination, bufor, (ssize_t) readSource);
    }
    close(source);
    close(destination);
}*/

mode_t read_chmod(char *source){
    struct stat mode;
    if(stat(source, &mode) != -1){
        printf("\nPobrano chmod\n");
        return mode.st_mode;
    }
    else exit EXIT_FAILURE;
}

time_t read_time(char *source){
    struct stat time;
    if(stat(source,&time) != -1){
        printf("\nPobrano datę\n");
        return time.st_mtime;
    }
    else exit EXIT_FAILURE;
}

off_t read_size(char *source){
    struct stat size;
    if(stat(source, &size) != -1){
        printf("\nPobrano rozmiar\n");
        return size.st_size;
    }
    else exit EXIT_FAILURE;
}


void copyFile(char *sourceFile, char *destinationFile) {
    struct stat stbuf;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (source < 0 || destination < 0){
        printf("Couldn't open the file");
        exit EXIT_FAILURE;
    }
    fstat(source, &stbuf);
    sendfile(destination, source, 0, stbuf.st_size);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(!chmod(destinationFile, source_chmod)==0){
        printf("\nPoprawnie nadano uprawnienia\n");
    }
    else exit EXIT_FAILURE;

    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(!utime(destinationFile, source_time)){
        printf("\nPoprawnie przeniesiono stempel czasowy\n");
    }
    else exit EXIT_FAILURE;
}

int main(int argc, char *argv[]){
    ListSourceFiles_type *head;
    head = (ListSourceFiles_type *)malloc(sizeof(ListSourceFiles_type));
    head=NULL;

    if(argc <= 2){
        printf("Too few arguments\n");
        return EXIT_FAILURE;
    }
    if(argc > 3){
        printf("To many arguments\n");
        return EXIT_FAILURE;
    }
    printf("Source: %s\n", argv[1]);
    printf("Destination: %s\n", argv[2]);
    char *source = argv[1];
    char *destination = argv[2];
    if (!isDirectoryExists(source)){
        return EXIT_FAILURE;
    }
    if(!isDirectoryExists(destination)) {
        return EXIT_FAILURE;
    }
    DIR *dp; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
    struct dirent *ep;
    dp = opendir (source);
    char *tmp;
    char *name;
    char *des;
    //strcpy(name, source);
    //strcat(name,"/");
    //puts(name);
    if (dp != NULL){
        while (ep = readdir (dp)){
            puts(ep->d_name);
            strcpy(name, source);
            strcat(name,"/");
            printf("%s%s\n",name,ep->d_name);
            if(isFileExists(strcat(name,ep->d_name))){
                puts("True");
                addSourceFile(&head, ep->d_name);

                strcpy(des,destination);
                strcat(des,"/");
                copyFile(strcat(des, ep->d_name));
            }
        }
        (void) closedir (dp);
    }
    else{
        perror ("Couldn't open the directory");
    }
    show(head);
    /* Our process ID and Session ID */
    pid_t pid, sid;
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit EXIT_FAILURE;
    }
    /* If we got a good PID, then
    we can exit the parent process. */
    if (pid > 0) {
        exit EXIT_SUCCESS;
    }
    /* Change the file mode mask */
    umask(0);
    /* Open any logs here */
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit EXIT_FAILURE;
    }
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit EXIT_FAILURE;
    }
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    /* Daemon-specific initialization goes here */
    /* The Big Loop */
    //while (1) {
    /* Do some task here ... */
    //sleep(); /* wait 30 seconds */
    //}
    exit EXIT_SUCCESS;
}
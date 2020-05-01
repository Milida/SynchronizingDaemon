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
#include <stdbool.h>
#define BUFF_SIZE 64

typedef struct al {
    char *name;
} allocation;

/*
** http://www.unixguide.net/unix/programming/2.5.shtml
** About locking mechanism...
*/

void handler(int signum){
    syslog(LOG_INFO,"Waking a deamon with a signal");
}

int main(int argc, char *argv[]) {
    unsigned int sleepTime = 300;
    bool recursive = false;
    int fileSize = 30;
    //daje tutaj, najwyżej się przeniesie
    openlog("Deamon synchronization", LOG_PID | LOG_NDELAY, LOG_USER);

    if (argc <= 2) { //#FIXME popraw  na getopt
        printf("Too few arguments\n");
        syslog(LOG_ERR, "Too few arguments");
        exit(EXIT_FAILURE);
    }
    if (argc == 4) {
        if (strcmp(argv[3], "-r")) {
            printf("Invalid number of arguments\n");
            syslog(LOG_ERR, "Invalid number of arguments");
            exit(EXIT_FAILURE);
        } else {
            recursive = true;
        }
    } else if (argc == 5) {
        if (!strcmp(argv[3], "s")) {
            sleepTime = atoi(argv[4]);
        } else if (!strcmp(argv[3], "d")) {
            fileSize = atoi(argv[4]);
        } else if (!strcmp(argv[3], "-rs")) {
            recursive = true;
            sleepTime = atoi(argv[4]);
        } else if (!strcmp(argv[3], "-rd")) {
            recursive = true;
            fileSize = atoi(argv[4]);
        } else puts("Nieprawidłowy argument");
    } else if (argc == 6) {
        if (!strcmp(argv[3], "sd")) {
            sleepTime = atoi(argv[4]);
            fileSize = atoi(argv[5]);
        } else if (!strcmp(argv[3], "-rsd")) {
            recursive = true;
            sleepTime = atoi(argv[4]);
            fileSize = atoi(argv[5]);
        } else puts("Nieprawidłowy argument");
    } else {
        puts("Too many arguments");
        syslog(LOG_ERR, "Too many arguments");
        exit(EXIT_FAILURE);
    }
    char *source = argv[1];
    char *destination = argv[2];
    if (!isDirectoryExists(source)) {
        syslog(LOG_ERR, "Source directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    if (!isDirectoryExists(destination)) {
        syslog(LOG_ERR, "Destination directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    /* Our process ID and Session ID */
    pid_t pid, sid;
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Incorrect child pid");
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
    we can exit the parent process. */
    if (pid > 0) {
        syslog(LOG_INFO, "Correct child pid");
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);
    /* Open any logs here */
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Incorrect SessionID");
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Couldn't change the current working directory");
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Daemon-specific initialization goes here */
    signal(SIGUSR1, handler);
    /* The Big Loop */
    while (1) {
        DIR *sourceDir; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
        struct dirent *ep;
        sourceDir = opendir(source);
        MyExample *name = (MyExample *) malloc(sizeof(MyExample));
        name->name = (char *) malloc(BUFF_SIZE);
        MyExample *des = (MyExample *) malloc(sizeof(MyExample));
        des->name = (char *) malloc(BUFF_SIZE);
        //strcpy(name, source);
        //strcat(name,"/");
        if (sourceDir != NULL) {
            while (ep = readdir(sourceDir)) {
                strcpy(name->name, source);
                strcat(name->name, "/");
                if (isFileExists(strcat(name->name, ep->d_name))) {
                    strcpy(des->name, destination);
                    strcat(des->name, "/");
                    if (read_size(name->name) < fileSize) {
                        copy_File(name->name, strcat(des->name, ep->d_name));
                    } else copyFile(name->name, strcat(des->name, ep->d_name));
                } else if (isDirectoryExists(name->name) && strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") &&
                           recursive) {
                    printf("To jest folder %s\n",
                           name->name); //#FIXME jeśli folder istnieje w miejscu docelowym to poprawić żeby tego nie kopiować
                    strcpy(des->name, destination);
                    strcat(des->name, "/");
                    mode_t source_chmod = read_chmod(name->name);
                    if (mkdir(strcat(des->name, ep->d_name), source_chmod)) { //do poprawienia jeśli
                        syslog(LOG_ERR, "Couldn't change the chmod of dir");
                        exit(EXIT_FAILURE);
                    }
                    struct utimbuf source_time;
                    source_time.modtime = read_time(name->name);
                    source_time.actime = time(NULL);
                    if (utime(des->name, &source_time)) {
                        syslog(LOG_ERR, "Couldn't change the modification time");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            (void) closedir(sourceDir);
        } else {
            syslog(LOG_ERR, "Couldn't open the source directory");
            perror("Couldn't open the directory");
        }

        DIR *desDir;
        struct dirent *epp;
        desDir = opendir(destination);
        MyExample *na = (MyExample *) malloc(sizeof(MyExample));
        na->name = (char *) malloc(BUFF_SIZE);
        MyExample *desti = (MyExample *) malloc(sizeof(MyExample));
        desti->name = (char *) malloc(BUFF_SIZE);

        if (desDir != NULL) {
            while (epp = readdir(desDir)) {
                strcpy(na->name, destination);
                strcat(na->name, "/");
                if (isFileExists(strcat(na->name, epp->d_name))) {
                    strcpy(desti->name, source);
                    strcat(desti->name, "/");
                    deleteFile(na->name, strcat(desti->name, epp->d_name));
                }
            }
            (void) closedir(desDir);
        } else {
            syslog(LOG_ERR, "Couldn't open the destination directory");
            perror("Couldn't open the directory");
        }
        free(name);
        free(des);
        free(na);
        free(desti);
        syslog(LOG_INFO, "Daemon goes to sleep");
        if ((sleep(sleepTime)) == 0)
            syslog(LOG_INFO, "Daemon wakes up");
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    closelog();
    exit(EXIT_SUCCESS);
}
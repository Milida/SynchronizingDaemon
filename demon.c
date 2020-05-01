#include "filelib.h"

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
        demonCp(source, destination, recursive, fileSize);
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
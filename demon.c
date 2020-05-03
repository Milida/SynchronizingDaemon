#include "filelib.h"

int main(int argc, char *argv[]) {
    unsigned int sleepTime = 300; //setting default options
    bool recursive = false;
    int fileSize = 50;
    int choice;
    char *source = argv[1];
    char *destination = argv[2];
    openlog("Daemon synchronization", LOG_PID | LOG_NDELAY, LOG_USER);
    if (!isDirectoryExists(source)) { //checking the source path
        puts("Source directory doesn't exist");
        syslog(LOG_ERR, "Source directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    if (!isDirectoryExists(destination)) { //checking the destination path
        puts("Destination directory doesn't exist");
        syslog(LOG_ERR, "Destination directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    if(argc <= 2){
        puts("Too few arguments");
        syslog(LOG_ERR,"Too few arguments")
    }
    while((choice = getopt(argc,argv,"rs:d:")) != -1){ //checking and setting options from user's choice
        switch(choice){
            case 'r':
                recursive = true;
                break;
            case 's':
                sleepTime = atoi(optarg);
                break;
            case 'd':
                fileSize = atoi(optarg);
                break;
            case ':':
                printf("Option -%c requires an operand\n",optarg);
            default:
                puts("No such option");
                syslog(LOG_ERR, "No such option");
                exit(EXIT_FAILURE);
        }
    }
    if(optind >= argc){
        puts("Missing an argument after options");
        syslog(LOG_ERR, "Missing");
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
#include "filelib.h"

int main(int argc, char *argv[]) {
    unsigned int sleepTime = 300; //setting default options
    bool recursive = false;
    int fileSize = 1024;
    int choice;
    char *source = argv[1];
    char *destination = argv[2];
    openlog("Daemon synchronization", LOG_PID | LOG_NDELAY, LOG_USER);
    if (!dirExists(source)) { //checking the source path
        puts("Source directory doesn't exist");
        syslog(LOG_ERR, "Source directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    if (!dirExists(destination)) { //checking the destination path
        puts("Destination directory doesn't exist");
        syslog(LOG_ERR, "Destination directory doesn't exist");
        exit(EXIT_FAILURE);
    }
    if(argc <= 2){
        puts("Too few arguments");
        syslog(LOG_ERR,"Too few arguments");
    }
    if(argc > 8){
        puts("Too many arguments");
        syslog(LOG_ERR, "Too many arguments");
        exit(EXIT_FAILURE);
    }
    while((choice = getopt(argc,argv,":rs:d:")) != -1){ //checking and setting options from user's choice
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
                puts("Missing an operand");
                syslog(LOG_ERR, "Missong an operand");
                exit(EXIT_FAILURE);
            default:
                puts("No such option");
                syslog(LOG_ERR, "No such option");
                exit(EXIT_FAILURE);
        }
    }

    pid_t pid, sid;
    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Incorrect child pid");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        syslog(LOG_INFO, "Correct child pid");
        exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Incorrect SessionID");
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Couldn't change the current working directory");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, handler);
    while (1) { //TODO jeśli demon zostanie obudzony sygnałem w trakcie swojego działania to niech działa dalej, jeśli poza to niech wtedy się budzi
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
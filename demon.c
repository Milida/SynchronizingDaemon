#include "filelib.h"

int main(int argc, char *argv[]) {
    unsigned int sleepTime = 300; //setting default sleep time
    bool recursive = false; //implicitly it doesn't synchronize recursively
    int fileSize = 1024; //default split size to divide copying method
    int choice;
    struct sigaction sa;
    sa.sa_handler = handler; //setting handler
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; //setting SE_RESTART flag
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
        exit(EXIT_FAILURE);
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
                if(atoi(optarg) <= 0){
                    puts("Invalid sleepTime operand");
                    syslog(LOG_ERR, "Invalid sleepTime argument");
                    exit(EXIT_FAILURE);
                }
                else
                    sleepTime = atoi(optarg);
                break;
            case 'd':
                fileSize = atoi(optarg);
                if(fileSize <= 0){
                    puts("Invalid fileSize operand");
                    syslog(LOG_ERR, "Invalid fileSize operand");
                    exit(EXIT_FAILURE);
                }
                break;
            case ':':
                puts("Missing an operand");
                syslog(LOG_ERR, "Missing an operand");
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

    if(sigaction(SIGUSR1, &sa, NULL) == -1)
        puts("Signal handling error\n");
    while (1) {
        demonCp(source, destination, recursive, fileSize); //daemon synchronizing function
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
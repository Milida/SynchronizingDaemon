#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

int isDirectoryExists(const char *path) //codeforwin.org
{
    struct stat stats;
    stat(path, &stats);

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    if(argv <= 2){
        printf("Too few arguments\n");
        return EXIT_FAILURE;
    }
    if(argv >3){
        printf("To many arguments\n");
        return EXIT_FAILURE;
    }

    printf("Source: %s\n", argv[1]);
    printf("Destination: %s\n", argv[2]);
    char *source = argv[1];
    char *destination = argv[2];
    if (isDirectoryExists(source)){
        printf("Directory exists at path '%s'\n", source);
        if(isDirectoryExists(destination))
            prints("Directory exists at path '%s'\n", destination)
        else{
            printf("Directory does not exist at path '%s'\n", destination);
            return EXIT_FAILURE;
        }
    }
    else
        printf("Directory does not exist at path '%s'\n", source);

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

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */

    /* The Big Loop */
    while (1) {
        /* Do some task here ... */

        sleep(); /* wait 30 seconds */
    }
    exit(EXIT_SUCCESS);
}
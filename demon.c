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

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    stat(path, &stats);

    // Check for file existence
    return S_ISDIR(stats.st_mode);
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);

    return S_ISREG(stats.st_mode);
}

int main(int argc, char *argv[]) {
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

    if (isDirectoryExists(source)){
        printf("Directory exists at path '%s'\n", source);
        if(isDirectoryExists(destination)) {
            printf("Directory exists at path '%s'\n", destination);
        }
        else{
            printf("Directory does not exist at path '%s'\n", destination);
            return EXIT_FAILURE;
        }
    }
    else {
        printf("Directory does not exist at path '%s'\n", source);
    }

    DIR *dp; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
    struct dirent *ep;

    dp = opendir (source);
    if (dp != NULL)
    {
        while (ep = readdir (dp))
            printf(ep->d_name);
            if(isFileExists(source + '/' + ep->d_name)){
                puts(" True");
            }
        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");


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
    //while (1) {
        /* Do some task here ... */

        //sleep(); /* wait 30 seconds */
    //}
    exit(EXIT_SUCCESS);
}
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

typedef struct ListSourceFiles{
    char *file;
    struct ListSourceFiles * next;
} ListSourceFiles_type;

void addSourceFile(ListSourceFiles_type **head, char *newFile){
    if(*head!=NULL){
        ListSourceFiles_type *current=*head;
        while (current->next != NULL){
            current = current->next;
        }
        current->next = (ListSourceFiles_type *)malloc(sizeof(ListSourceFiles_type));
        current->next->file = newFile;
        current->next->next = NULL;
    }
    else{
        *head = (ListSourceFiles_type *)malloc(sizeof(ListSourceFiles_type));
        (*head)->file = newFile;
        (*head)->next = NULL;
    }
}
void show(ListSourceFiles_type *head)
{
    printf("\n");
    if(head==NULL) printf("List is empty");
    else{
        ListSourceFiles_type *current=head;
        do {
            printf("%s", current->file);
            printf("\n");
            current = current->next;
        }while (current != NULL);

    }
}

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    stat(path, &stats);
    // Check for file existence
    return (S_ISDIR(stats.st_mode));
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);
    return S_ISREG(stats.st_mode);
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
    char *tmp;
    char *name;
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
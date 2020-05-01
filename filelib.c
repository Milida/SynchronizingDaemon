#include "filelib.h"

void handler(int signum){
    syslog(LOG_INFO,"Waking a deamon with a signal");
}

int isDirectoryExists(const char *path){ //codeforwin.org
    struct stat stats;
    if(stat(path, &stats) != -1)
        return S_ISDIR(stats.st_mode);
    return 0;
}

int isFileExists(const char *path){
    struct stat stats;
    stat(path, &stats);
    if(stat(path,&stats) != -1){
        return S_ISREG(stats.st_mode);
    }
    return 0;
}
mode_t read_chmod(char *source){
    struct stat mode;
    if(stat(source, &mode) != -1)
        return mode.st_mode;
    else {
        syslog(LOG_ERR,"Couldn't take a file chmod");
        exit(EXIT_FAILURE);
    }
}

time_t read_time(char *source){
    struct stat time;
    if(stat(source,&time) != -1)
        return time.st_mtime;
    else{
        syslog(LOG_ERR,"Couldn't take a modification time");
        exit(EXIT_FAILURE);
    }
}

off_t read_size(char *source){
    struct stat size;
    if(stat(source, &size) != -1)
        return size.st_size;
    else{
        syslog(LOG_ERR,"Couldn't take a file size");
        exit(EXIT_FAILURE);
    }
}

void copyFile(char *sourceFile, char *destinationFile) {
    char bufor[4096]; //małe
    int readSource, writeDes; //małe
    struct stat stbuf;
    int source = open(sourceFile, O_RDONLY);
    int destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if(destination < 0 && errno == EEXIST){
        //printf("Plik %s już istnieje\n", sourceFile);
        if(read_time(sourceFile) == read_time(destinationFile)) return;
    }
    destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
    if ((source < 0 || destination < 0) && errno != EEXIST) {
       // printf("Couldn't open the file");
        syslog(LOG_ERR,"Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    if (read_size(sourceFile) < fileSize) {
        while ((readSource = read(source, bufor, sizeof(bufor))) > 0){
            writeDes = write(destination, bufor, (ssize_t) readSource);
        }
    }
    else {
        //printf("Copying %s file\n",sourceFile);
        if (fstat(source, &stbuf) == -1) {
            // printf("Fstat errno %d\n",errno);
            syslog(LOG_ERR, "Couldn't copy the file");
        }
        if (sendfile(destination, source, 0, stbuf.st_size) == -1) {
            //printf("%d\n",errno);
            // printf("Couldn't copy the file\n");
            syslog(LOG_ERR, "Couldn't copy the file");
        }
    }
    syslog(LOG_INFO,"File copied successfully: %s", destinationFile);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){
        syslog(LOG_ERR,"Couldn't change the chmod");
        exit(EXIT_FAILURE);
    }

    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(utime(destinationFile, &source_time)){
        syslog(LOG_ERR,"Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void deleteFile(char *destinationFile, char *sourceFile) {
    int destination = open(destinationFile, O_RDONLY);
    int source = open(sourceFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if (source < 0 && errno == EEXIST) {
        printf("Plik %s już istnieje  %d\n", sourceFile, errno);
        return;
    } else if (remove(destinationFile) == 0){
        printf("\nDeleted successfully %s\n", destinationFile);
        syslog(LOG_INFO,"File deleted successfully");
    }
    remove(sourceFile);

    close(source);
    close(destination);
}

void demonCp (static char *source, static char *destination, bool recursive, int fileSize){
    DIR *sourceDir; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
    struct dirent *ep;
    sourceDir = opendir(source);
    allocation *name = (allocation *) malloc(sizeof(allocation));
    name->name = (char *) malloc(BUFF_SIZE);
    allocation *des = (allocation *) malloc(sizeof(allocation));
    des->name = (char *) malloc(BUFF_SIZE);
    if (sourceDir != NULL) {
        while (ep = readdir(sourceDir)) {
            strcpy(name->name, source);
            strcat(name->name, "/");
            if (isFileExists(strcat(name->name, ep->d_name))) {
                strcpy(des->name, destination);
                strcat(des->name, "/");
                copyFile(name->name, strcat(des->name, ep->d_name), fileSize)
            } else if (isDirectoryExists(name->name) && strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") && recursive) {
                printf("To jest folder %s\n",name->name); //#FIXME jeśli folder istnieje w miejscu docelowym to poprawić żeby tego nie kopiować
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
    allocation *na = (allocation *) malloc(sizeof(allocation));
    na->name = (char *) malloc(BUFF_SIZE);
    allocation *desti = (allocation *) malloc(sizeof(allocation));
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
}
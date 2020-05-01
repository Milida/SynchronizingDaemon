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

void copyFile(char *sourceFile, char *destinationFile, int fileSize) {
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
        printf("Couldn't open the file %d\n",errno);
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

void copyDir(char *source, char *destination, bool recursive, int fileSize){
    mode_t source_chmod = read_chmod(source);
    if (mkdir(destination, source_chmod)) { //do poprawienia jeśli
        if(errno != EEXIST){
            printf("%d\n",errno);
            syslog(LOG_ERR, "Couldn't change the chmod of dir");
            exit(EXIT_FAILURE);
        }
    }
    demonCp(source, destination, recursive, fileSize);
    struct utimbuf source_time;
    source_time.modtime = read_time(source);
    source_time.actime = time(NULL);
    if (utime(destination, &source_time)) {
        syslog(LOG_ERR, "Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void demonCp (char *source, char *destination, bool recursive, int fileSize){
    DIR *sourceDir; //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
    struct dirent *ep;
    sourceDir = opendir(source);
    char *name = (char*)malloc(sizeof(char));
    char *des = (char *)malloc(sizeof(char));
    if (sourceDir != NULL) {
        while (ep = readdir(sourceDir)) {
            name = catDir(name, source, ep->d_name);
            //strcpy(name->name, source);
            //strcat(name->name, "/");
            if (isFileExists(name)) {
                puts(destination);
                des = catDir(des, destination,ep->d_name);
                //strcpy(des->name, destination);
                //strcat(des->name, "/");
                copyFile(name, des, fileSize);
                //free(des);
            } else if (isDirectoryExists(name) && strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") && recursive) {
                des = catDir(des, destination, ep->d_name);
                //strcpy(des->name, destination);
                //strcat(des->name, "/");
                copyDir(name, des, recursive, fileSize);
                //free(des);
            }
            //free(name);
        }
        (void) closedir(sourceDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the source directory");
        perror("Couldn't open the directory");
    }
    deleteFromDir(source, destination);
    free(name);
    free(des);
}

void deleteFromDir(char *source, char *destination){
    DIR *desDir;
    struct dirent *epp;
    desDir = opendir(destination);
    char *na = (char *)malloc(sizeof(char));
    char *desti = (char *)malloc(sizeof(char));

    if (desDir != NULL) {
        while (epp = readdir(desDir)) {
            na = catDir(na,destination,epp->d_name);
            if (isFileExists(na)) {
                desti = catDir(desti,source, epp->d_name);
                deleteFile(na, desti);
                //free(desti);
            }
            //free(na);
        }
        (void) closedir(desDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the destination directory");
        perror("Couldn't open the directory");
    }
    free(na);
    free(desti);
}

char *catDir(char* newptr, char *first, char *second){
    newptr = realloc(newptr,strlen(first)+strlen(second)+2);
    strcpy(newptr,first);
    strcat(newptr,"/");
    strcat(newptr,second);
    return newptr;
}
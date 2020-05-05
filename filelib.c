#include "filelib.h"

void handler(int signum){ //signal handler
    syslog(LOG_INFO,"Waking a daemon with a signal");
}

int dirExists(const char *path) {//checking if directory exists
    struct stat stats;
    if(stat(path, &stats) != -1)
        return S_ISDIR(stats.st_mode);
    return 0;
}

int fileExists(const char *path) {//checking if regular file exists
    struct stat stats;
    stat(path, &stats);
    if(stat(path,&stats) != -1)
        return S_ISREG(stats.st_mode);
    return 0;
}

mode_t read_chmod(char *source) {//reading chmode
    struct stat mode;
    if(stat(source, &mode) != -1)
        return mode.st_mode;
    else {
        syslog(LOG_ERR,"Couldn't take a file chmod");
        exit(EXIT_FAILURE);
    }
}

time_t read_time(char *source) {//reading modification timestamp
    struct stat time;
    if(stat(source,&time) != -1)
        return time.st_mtime;
    else{
        syslog(LOG_ERR,"Couldn't take a modification time");
        exit(EXIT_FAILURE);
    }
}

off_t read_size(char *source) {//reading size
    struct stat size;
    if(stat(source, &size) != -1)
        return size.st_size;
    else{
        syslog(LOG_ERR,"Couldn't take a file size");
        exit(EXIT_FAILURE);
    }
}

void copyFile(char *sourceFile, char *destinationFile, int fileSize) {//copying file
    int source = open(sourceFile, O_RDONLY);
    int destination;
    if(fileExists(destinationFile)){ //file already exists
        destination = open(destinationFile, O_WRONLY | O_TRUNC, 0644);
        if(read_time(sourceFile) == read_time(destinationFile)) //if timespamps are qual we don't have to copy
            return;
    }
    else
        destination = open(destinationFile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);//creating and opening file
    if (source < 0 || destination < 0) {
        syslog(LOG_ERR,"Couldn't open the file");
        exit(EXIT_FAILURE);
    }
    if (read_size(sourceFile) < fileSize) {//copying small files with read/write
        char buffer[4096];
        int readSource, writeDes;
        while ((readSource = read(source, buffer, sizeof(buffer))) > 0)
            writeDes = write(destination, buffer, (ssize_t) readSource);
    }
    else {//copying big files with sendfile
        struct stat statbuf;
        if (fstat(source, &statbuf) == -1)
            syslog(LOG_ERR, "Couldn't copy the file");
        else if (sendfile(destination, source, 0, statbuf.st_size) == -1)
            syslog(LOG_ERR, "Couldn't copy the file");
    }
    syslog(LOG_INFO,"File copied successfully: %s", destinationFile);

    close(source);
    close(destination);

    mode_t source_chmod = read_chmod(sourceFile);
    if(chmod(destinationFile, source_chmod)){//changing chmod
        syslog(LOG_ERR,"Couldn't change the chmod");
        exit(EXIT_FAILURE);
    }

    struct utimbuf source_time;
    source_time.modtime = read_time(sourceFile);
    source_time.actime = time(NULL);
    if(utime(destinationFile, &source_time)){//changing modification timestamp
        syslog(LOG_ERR,"Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void copyDir(char *source, char *destination, bool recursive, int fileSize) {//copying directory and its contents
    mode_t source_chmod = read_chmod(source);
    if (mkdir(destination, source_chmod)) {
        if (errno != EEXIST) {
            syslog(LOG_ERR, "Couldn't change the chmod of dir");
            exit(EXIT_FAILURE);
        }
    }
    demonCp(source, destination, recursive, fileSize);//copying contents of directory
    struct utimbuf source_time;
    source_time.modtime = read_time(source);
    source_time.actime = time(NULL);
    if (utime(destination, &source_time)) {//changing modification timestamp
        syslog(LOG_ERR, "Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void demonCp (char *source, char *destination, bool recursive, int fileSize) {
    DIR *sourceDir;
    struct dirent *ep;
    sourceDir = opendir(source);
    char *name = (char *)malloc(sizeof(char));
    char *des = (char *)malloc(sizeof(char));
    if (sourceDir != NULL) {
        while (ep = readdir(sourceDir)) {
            name = catDir(name, source, ep->d_name);
            if (fileExists(name)) {
                des = catDir(des, destination,ep->d_name);
                copyFile(name, des, fileSize);
            } else if (dirExists(name) && strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") && recursive) {
                des = catDir(des, destination, ep->d_name);
                copyDir(name, des, recursive, fileSize);
            }
        }
        (void) closedir(sourceDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the source directory");
        perror("Couldn't open the directory");
    }
    deleteFromDir(source, destination, recursive);
    free(name);
    free(des);
}

void deleteFromDir(char *source, char *destination, bool recursive) {
    DIR *desDir;
    struct dirent *epp;
    desDir = opendir(destination);
    char *src = (char *)malloc(sizeof(char));
    char *dst= (char *)malloc(sizeof(char));

    if (desDir != NULL) {
        while (epp = readdir(desDir)) {
            dst = catDir(dst,destination,epp->d_name);
            src = catDir(src,source, epp->d_name);
            if (fileExists(dst) && !fileExists(src)) {
                if(remove(dst)==0)
                    syslog(LOG_INFO,"File deleted successfully %s", dst);
            }
            else if(recursive && (dirExists(dst) && !dirExists(src)) && strcmp(epp->d_name,".") && strcmp(epp->d_name,"..")){
                deleteFromDir(src, dst, recursive);
                if(!rmdir(dst))
                    syslog(LOG_INFO,"Directory deleted successfully %s",dst);
            }
        }
        (void) closedir(desDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the destination directory %s",destination);
        perror("Couldn't open the directory");
    }
    free(src);
    free(dst);
}

char *catDir(char* newptr, char *first, char *second) {
    newptr = realloc(newptr,strlen(first)+strlen(second)+2);
    strcpy(newptr,first);
    strcat(newptr,"/");
    strcat(newptr,second);
    return newptr;
}
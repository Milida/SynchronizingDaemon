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
        if(read_time(sourceFile) == read_time(destinationFile)) //if timestamps are equal we don't have to copy
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
        int readSource;
        while ((readSource = read(source, buffer, sizeof(buffer))) > 0)
            write(destination, buffer, (ssize_t) readSource);
    }
    else {//copying big files with sendfile
        struct stat statbuff;
        if (fstat(source, &statbuff) == -1)
            syslog(LOG_ERR, "Couldn't copy the file");
        else if (sendfile(destination, source, 0, statbuff.st_size) == -1)
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

void copyDir(char *source, char *destination, int fileSize) {//copying directory and its contents
    mode_t source_chmod = read_chmod(source);
    if (mkdir(destination, source_chmod)) {
        if (errno != EEXIST) {
            syslog(LOG_ERR, "Couldn't change the chmod of dir");
            exit(EXIT_FAILURE);
        }
    }
    demonCp(source, destination, 1, fileSize);//copying contents of directory
    struct utimbuf source_time;
    source_time.modtime = read_time(source);
    source_time.actime = time(NULL);
    if (utime(destination, &source_time)) {//changing modification timestamp
        syslog(LOG_ERR, "Couldn't change the modification time");
        exit(EXIT_FAILURE);
    }
}

void demonCp (char *source, char *destination, bool recursive, int fileSize) {//synchronizing two directories
    DIR *sourceDir;
    struct dirent *dir;
    sourceDir = opendir(source);
    char *src = (char *)malloc(sizeof(char));
    char *des = (char *)malloc(sizeof(char));
    if (sourceDir != NULL) {
        while (dir = readdir(sourceDir)) {
            src = catDir(src, source, dir->d_name);
            if (fileExists(src))
                des = catDir(des, destination,dir->d_name);
                copyFile(src, des, fileSize);//copying regular file
            } else if (dirExists(src) && strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && recursive) {
                des = catDir(des, destination, dir->d_name);
                copyDir(src, des, fileSize);//copying directory with contents
            }
        }
        (void) closedir(sourceDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the source directory");
        exit(EXIT_FAILURE);
    }
    deleteFromDir(source, destination, recursive);//deleting unnecessary files from destination directory
    free(src);
    free(des);
}

void deleteFromDir(char *source, char *destination, bool recursive) {//deleting unnecessary files from destination directory
    DIR *desDir;
    struct dirent *dir;
    desDir = opendir(destination);
    char *src = (char *)malloc(sizeof(char));
    char *dst= (char *)malloc(sizeof(char));
    if (desDir != NULL) {
        while (dir = readdir(desDir)) {
            dst = catDir(dst,destination,dir->d_name);
            src = catDir(src,source, dir->d_name);
            if (fileExists(dst) && !fileExists(src)) {
                if(remove(dst)==0)
                    syslog(LOG_INFO,"File deleted successfully %s", dst);
            }
            else if(recursive && dirExists(dst) && !dirExists(src) && strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
                deleteFromDir(src, dst, recursive);
                if(!rmdir(dst))
                    syslog(LOG_INFO,"Directory deleted successfully %s",dst);
            }
        }
        (void) closedir(desDir);
    } else {
        syslog(LOG_ERR, "Couldn't open the destination directory %s",destination);
        exit(EXIT_FAILURE);
    }
    free(src);
    free(dst);
}

char *catDir(char* ptr, char *path, char *name) { //adding name to the path
    ptr = realloc(ptr,strlen(path)+strlen(name)+2);
    strcpy(ptr,path);
    strcat(ptr,"/");
    strcat(ptr,name);
    return ptr;
}
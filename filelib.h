#ifndef filelib
#define filelib
#include <sys/types.h>

int isDirectoryExists(const char *path);
int isFileExists(const char *path);
mode_t read_chmod(char *source);
time_t read_time(char *source);
off_t read_size(char *source);
void copy_File(char *sourceFile, char *destinationFile);
void copyFile(char *sourceFile, char *destinationFile);
void deleteFile(char *destinationFile, char *sourceFile);

#endif //filelib
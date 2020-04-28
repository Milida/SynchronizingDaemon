#ifndef filelist
#define filelist

typedef struct ListSourceFiles{
    char *file;
    struct ListSourceFiles * next;
} ListSourceFiles_type;

void addSourceFile(ListSourceFiles_type **head, char *newFile);
void show(ListSourceFiles_type *head);

#endif //filelist

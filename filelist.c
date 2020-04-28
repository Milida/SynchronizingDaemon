#include <stdio.h>
#include <stdlib.h>
#include "filelist.h"

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

void show(ListSourceFiles_type *head){
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
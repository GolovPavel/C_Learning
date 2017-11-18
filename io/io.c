#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1024

void fprintend(FILE*, char*);
void getlstln(FILE*, char*);

int main (int argc, char* argv[]){
    char* file_name = argv[1];

    FILE* fm = fopen(file_name, "r+");

   /*  fprintend(fm, "1\n"); */

    char lstln[MAXLINE];
    getlstln(fm, lstln);
    printf("%s\n", lstln);

    exit(0);
}

void fprintend(FILE* fm, char* str){
    fseek(fm, 0, SEEK_END);
    fputs(str, fm);
}

void getlstln(FILE* fm, char* lstln){
  char buf[MAXLINE];
  fseek(fm, 0, SEEK_SET);

  while(fgets(buf, MAXLINE, fm) != NULL) {
    strncpy(lstln, buf, MAXLINE);
  }
}

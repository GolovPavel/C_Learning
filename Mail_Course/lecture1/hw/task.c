#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>

#define MAXLINE 1024

void getlstln(FILE*, char*);


int main (int argc, char* argv[]){
 char* file = argv[1];
 int fd;
 struct flock lock;

 printf ("opening %s\n", file);

 /* Open a file descriptor to the file. */
 FILE* fm = fopen(file, "r+");
 fd = fileno(fm);

 /* Initialize the flock structure. */
 memset(&lock, 0, sizeof(lock));
 lock.l_type = F_RDLCK;

 /* Place a read lock on the file. */
 fcntl (fd, F_SETLKW, &lock);

 /* Add new number to file */
 fseek(fm, 0, SEEK_END);
 long size = ftell(fm);

 if (size == 0) {
    fseek(fm, 0, SEEK_SET);
    fprintf(fm, "%d\n", 1);
 } else {
    char lstln[MAXLINE];
    getlstln(fm, lstln);

    /* Inc number */
    int num = atoi(lstln);
    num++;
    fseek(fm, 0, SEEK_END);
    fprintf(fm, "%d\n", num);
 }

 /* Release the lock. */
 lock.l_type = F_UNLCK;
 fcntl (fd, F_SETLKW, &lock);

 printf("Lock realeased");
 exit(0);
}

void getlstln(FILE* fm, char* lstln){
 char buf[MAXLINE];
 fseek(fm, 0, SEEK_SET);

 while(fgets(buf, MAXLINE, fm) != NULL) {
  strncpy(lstln, buf, MAXLINE);
 }
}

#include <stdio.h>

void strcat(char* buff, char* add);

int main(int argc, char *argv[]){
    char buff[20];

    buff[0] = 'A';
    buff[1] = '\0';

    char *add = "asdas";

    strcat(buff, add);

    printf("%s\n", buff);
}

void strcat(char* buff, char* add) {
    while(*buff != '\0') {
        buff++;
    }

    while((*buff = *add) != '\0'){
        buff++;
        add++;
    };
}

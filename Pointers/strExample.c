#include <stdio.h>

int strlen(char*);

int main(int argc, char *argv[]) {
    char *str = *(argv + 1);

    printf("%d\n", strlen(str));
}

int strlen(char *str){
    int len;
    for(len = 0; *str != '\0'; str++) {
        len++;
    }
    return len;
}

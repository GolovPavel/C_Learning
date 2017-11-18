#include <stdio.h>
#include <string.h>

void strcopy(char*, char*);


int main() {
    char* s1 = "Hello!";
    char s2[20];

    strcopy(s1, s2);

    *s2 = 'A';

    printf("%s\n", s1);
    printf("%s\n", s2);
}

void strcopy(char* src, char* dest) {
    while(*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
}


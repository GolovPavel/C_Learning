#include <stdio.h>

int main(){
    char buff[2];

    char *point;
    char *point1;

    point = &buff;
    
    buff[0] = 'A';
    buff[1] = 'B';

    printf("%c\n", *point);
    printf("%c\n", *(point + 1));
    
    point1 = point;
    *point1 = 'N';

    printf("%c, %c\n", *point, *(point + 1));
    return 0;
}

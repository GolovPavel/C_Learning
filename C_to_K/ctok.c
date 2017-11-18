#include <stdio.h>

#define STEP 0.1
#define START_DEGREE 10

int main(){
    float t_k, t_c;
    t_c = START_DEGREE;

    while(t_c < 15 + STEP) {
        t_k = t_c + 273;
        printf("%3.1f\t%3.1f\n", t_c, t_k);
        t_c = t_c + STEP;
    }
    return 0;
}

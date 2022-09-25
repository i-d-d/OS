#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    float init_num = 0, div_num = 0;

    scanf("%f", &init_num);

    while (scanf("%f", &div_num) != EOF) {
        
        if (div_num == 0) {
            exit(EXIT_FAILURE);
        }

        init_num /= div_num;
    }
    
    write(1, &init_num, sizeof(init_num));
    return 0;
}
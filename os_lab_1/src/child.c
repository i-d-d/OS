#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define STR_LEN 32

int read_float(int fd, float *f) {
    char *buf = calloc(sizeof(char), STR_LEN);
    char c = 0;
    short i = 0;
    if (read(fd, &c, 1) < 0) {
        exit(3);
    }
    if ((c == ' ') || (c == '\n') || (c == '\0'))
        return 0;
    while (c != ' ' && c != '\n') {
        buf[i++] = c;
        read(fd, &c, 1);
    }


    *f = strtof(buf, NULL);
    return 1;
}

int main() {
    float init_num = 0, div_num = 0;

    if (read_float(0, &init_num) < 1) {
        exit(4);
    }

    while (read_float(0, &div_num) != 0) {
        
        if (div_num == 0) {
            exit(1);
        }

        init_num /= div_num;
    }
    
    if (write(1, &init_num, sizeof(init_num)) != sizeof(init_num)) {
        exit(2);
    }

    return 0;
}
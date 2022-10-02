#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define STR_LEN 32

int first = 1;

int read_float(int fd, float *f) {
    char *buf = calloc(sizeof(char), STR_LEN);
    char c = 0;
    short i = 0;

    if (read(fd, &c, 1) < 0) {
        exit(3);
    }

    if ((c == ' ') || (c == '\n') || (c == '\0'))
        return -1;

    while (c != ' ' && c != '\n') {
        buf[i++] = c;
        read(fd, &c, 1);
    }

    *f = strtof(buf, NULL);

    if (c == '\n') {
        return 0;
    }
    
    return 1;
}

int exec_command() {
    float init_num = 0, div_num = 0;

    if (read_float(0, &init_num) == -1) {

        if (first == 1)
            exit(4);
        else
            return 0;
    }

    else {
        first = 0;
    }

    int read_result;

    while ((read_result = read_float(0, &div_num)) != -1) {
        
        if (div_num == 0) {
            exit(1);
        }

        init_num /= div_num;

        if (read_result == 0) {
            break;
        }
    }
    
    if (write(1, &init_num, sizeof(float)) < 0) {
        exit(2);
    }

    return 1;
}

int main() {
    while (exec_command() == 1) {
        ;
    }

    char c = '\0';
    if (write(1, &c, sizeof(char)) < 0) {
        exit(2);
    }

    return 0;
}
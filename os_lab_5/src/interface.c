#include "./headers/interface.h"
#include "./lib/functions.h"
#include <unistd.h>

int read_number(int *result) {
    char *buf = calloc(sizeof(char), STR_LEN);
    char c = 0;
    short i = 0;

    if (read(0, &c, 1) < 0) {
        exit(1);
    }
    if ((c == ' ') || (c == '\n') || (c == '\0'))
        return -1;
    while (c != ' ' && c != '\n') {
        buf[i++] = c;
        read(0, &c, 1);
    }
    *result = atoi(buf);
    free(buf);
    if (c == '\n') {
        return 0;
    }
    return 1;
}

command get_command() {
    int command_number;
    read_number(&command_number);
    switch (command_number) {
        case 0:
            return CHANGE_IMP;
        case 1:
            return EXEC_1;
        case 2:
            return EXEC_2;
        case -1:
            return EXIT;
        default:
            return UNKNOWN;
    }
}

void execute_command(command com) {
    switch (com) {
        case EXEC_1:
            double a, b, e;
            scanf("%lf %lf %lf", &a, &b, &e);
            double result = sin_integral(a, b, e);
            printf("%lf\n\n", result);
            break;
        case EXEC_2:
            int curr, status, i = 0, arr_size = ARR_SIZE_INIT;
            int *arr = malloc(arr_size * sizeof(int));
            do {
                status = read_number(&curr);
                if (status == -1) {
                    break;
                }
                arr[i++] = curr;
                if (i == arr_size) {
                    arr_size *= 2;
                    arr = realloc(arr, arr_size * sizeof(int));
                }
            } while (status != 0);
            arr = realloc(arr, i * sizeof(int));
            int *sorted = sort(arr, i);
            free(arr);
            for (int j = 0; j < i; ++j) {
                printf("%d ", sorted[j]);
            }
            printf("\n\n");
            break;
        case UNKNOWN:
            printf("Invalid command!\n");
            break;
        case EXIT:
            break;
    }
}
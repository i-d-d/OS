#include <unistd.h>
#include <dlfcn.h>

#include "./headers/interface.h"

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

int main() {
    command input;
    int implementation = 1;
    void *handle = dlopen("/home/iddqd/Projects/C/OS/os_lab_5/src/lib/libFIRST.so", RTLD_LAZY);
    double(*sin_integral)(double, double, double) = dlsym(handle, "sin_integral");
    int*(*sort)(int*, int) = dlsym(handle, "sort");

    printf("\nYou are using implementation number %d\n\n", implementation);

    do {
        input = get_command();
        if (input == CHANGE_IMP) {
            if (implementation == 1) {
                dlclose(handle);
                implementation = 2;
                handle = dlopen("/home/iddqd/Projects/C/OS/os_lab_5/src/lib/libSECOND.so", RTLD_LAZY);
                sin_integral = dlsym(handle, "sin_integral");
                sort = dlsym(handle, "sort");
            }
            else {
                dlclose(handle);
                implementation = 1;
                handle = dlopen("/home/iddqd/Projects/C/OS/os_lab_5/src/lib/libFIRST.so", RTLD_LAZY);
                sin_integral = dlsym(handle, "sin_integral");
                sort = dlsym(handle, "sort");
            }
            printf("\nYou are using implementation number %d\n\n", implementation);
        }
        switch (input) {
            case EXEC_1:
                double a, b, e;
                scanf("%lf %lf %lf", &a, &b, &e);
                double result = (*sin_integral)(a, b, e);
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
                int *sorted = (*sort)(arr, i);
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
    } while (input != EXIT);
    dlclose(handle);
    return 0;
}
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STR_LEN 128
#define MAX_LENGTH 100

int first = 1;
int ind = 0;

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

int exec_command(float *result) {

    int read_result;

    float init_num = 0, div_num = 0;

    if ((read_result = read_float(0, &init_num)) == -1) {

        if (first == 1)
            exit(4);
        else
            return 0;
    }

    else if (read_result == 0) {
        if (ind >= MAX_LENGTH) {
            exit(6);
        }
        result[ind++] = init_num;
        first = 0;
    }

    else {
        first = 0;
        while ((read_result = read_float(0, &div_num)) != -1) {
            
            if (div_num == 0) {
                exit(1);
            }

            init_num /= div_num;

            if (read_result == 0) {
                break;
            }
        }
        if (ind >= MAX_LENGTH) {
            exit(6);
        }
        result[ind++] = init_num;

    }

    return 1;
}

size_t strlen_new(char *s) {
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] == '\n')
            return i;
    }
    return strlen(s);
}

int main() {

    char *fname_buf = calloc(sizeof(char), STR_LEN);
    if (read(0, fname_buf, STR_LEN) < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    char *fname = malloc(sizeof(char) * strlen_new(fname_buf));
    strncpy(fname, fname_buf, strlen_new(fname_buf));
    free(fname_buf);

    void *shared = mmap(NULL, sizeof(int) + sizeof(float) * MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        perror("Mapping error");
        exit(EXIT_FAILURE);
    }

    int filedes;
    if ((filedes = open(fname, O_RDONLY)) < 0) {
        perror(fname);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {     // Child

        float *shared_arr = (float *) (shared + sizeof(int));
        if (dup2(filedes, 0) < 0) {
            perror("Descriptor redirection error (input)");
            exit(EXIT_FAILURE);
        }

        while (exec_command(shared_arr) == 1) {
            ;
        }

        * (int *) shared = ind;

        if (munmap(shared, MAX_LENGTH) < 0) {
            perror("mumap");
            exit(EXIT_FAILURE);
        }
        if (close(filedes) < 0) {
            perror("Close error");
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    else {              // Parent

        int child_exit_status;
        if (waitpid(pid, &child_exit_status, 0) < 0) {
            perror("Waitpid error");
        }

        if (WEXITSTATUS(child_exit_status) == 1) {
            char *err1 = "Child process error exit, divison by zero\n";
            write(2, err1, strlen(err1));
            exit(EXIT_FAILURE);
        }

        else if (WEXITSTATUS(child_exit_status) == 4) {
            char *err2 = "Child process error exit, expected float number\n";
            write(2, err2, strlen(err2));
            exit(EXIT_FAILURE);
        }

        else if (WEXITSTATUS(child_exit_status) == 6) {
            char *err3 = "Child process error exit, too many strings\n";
            write(2, err3, strlen(err3));
            exit(EXIT_FAILURE);
        }

        else if (WEXITSTATUS(child_exit_status) != 0) {
            perror("Child process error");
            exit(EXIT_FAILURE);
        }

        int number = * (int *) shared;
        float result;
        float *shared_arr = (float *) (shared + sizeof(int));

        for (int i = 0; i < number; ++i) {
            result = shared_arr[i];
            char *result_string = calloc(sizeof(char), STR_LEN);
            gcvt(result, 7, result_string);
            if (write(1, result_string, strlen(result_string)) < 0) {
                perror("Can't write result to stdout");
                exit(EXIT_FAILURE);
            }

            char endline_c = '\n';
            if (write(1, &endline_c, 1) < 0) {
                perror("Can't write \\n to stdout");
                exit(EXIT_FAILURE);
            }
        }

        if (munmap(shared, MAX_LENGTH) < 0) {
            perror("Munmap error");
            exit(EXIT_FAILURE);
        }
        if (close(filedes) < 0) {
            perror("Close error");
            exit(EXIT_FAILURE);
        }
        
    }

    return 0;
}
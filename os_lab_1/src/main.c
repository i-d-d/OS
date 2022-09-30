#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STR_LEN 128

char *const child_args[] = { "child", NULL };
char *child_env[] = { NULL };

size_t strlen_new(char *s) {
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] == '\n')
            return i;
    }
    return strlen(s);
}

int main() {

    char *fname_buf = calloc(sizeof(char), STR_LEN);
    read(0, fname_buf, STR_LEN);

    char *fname = malloc(sizeof(char) * strlen_new(fname_buf));
    strncpy(fname, fname_buf, strlen_new(fname_buf));
    free(fname_buf);

    pid_t fd[2];
    pipe(fd);

    int filedes;
    if ((filedes = open(fname, O_RDONLY)) < 0) {
        perror(fname);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {     // Child

        close(fd[0]);
        if (dup2(filedes, 0) < 0) {
            perror("Descriptor redirection error (input)");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd[1], 1) < 0) {
            perror("Descriptor redirection error (output)");
            exit(EXIT_FAILURE);
        }
        execve(child_args[0], child_args, child_env);
        perror("execve");
    }

    else {              // Parent

        int child_exit_status;
        if (waitpid(pid, &child_exit_status, 0) < 0) {
            perror("waitpid");
        }
        if (WEXITSTATUS(child_exit_status) == 1) {
            char *err1 = "Child process error exit, divison by zero\n";
            write(2, err1, strlen(err1));
            exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(child_exit_status) == 4) {
            char *err2 = "Child process error exit, expected float number\n";
            write(2, err2, strlen(err2));
            exit(EXIT_FAILURE);
        }

        else if (WEXITSTATUS(child_exit_status) != 0) {
            perror("Child process error");
            exit(EXIT_FAILURE);
        }

        float result;
        if (read(fd[0], &result, sizeof(result)) < 0) {
            perror("Can't read from pipe");
            exit(EXIT_FAILURE);
        }

        close(fd[0]);
        close(fd[1]);
        close(filedes);

        char *result_string = calloc(sizeof(char), STR_LEN);
        gcvt(result, 7, result_string);
        if (write(1, result_string, strlen(result_string)) < 0) {
            perror("Can't write result to 1");
            exit(EXIT_FAILURE);
        }

        char endline_c = '\n';
        if (write(1, &endline_c, 1) < 0) {
            perror("Can't write \\n to 1");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
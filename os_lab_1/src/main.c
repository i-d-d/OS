#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#define STR_LEN 128


int main() {

    char *const child_args[] = { "child", NULL };
    char *child_env[] = { NULL };

    char fname_buf[STR_LEN];
    memset(fname_buf, '\0', STR_LEN);
    read(0, fname_buf, STR_LEN);

    char *fname = malloc(sizeof(char) * strlen(fname_buf));
    strncpy(fname, fname_buf, strlen(fname_buf));

    pid_t fd[2];
    pipe(fd);

    int filedes;
    if ((filedes = open(fname, O_RDONLY)) < 0) {
        perror(fname);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork() error");
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
        perror("execve()");
    }

    else {              // Parent

        int child_exit_status;
        waitpid(pid, &child_exit_status, 0);
        if (!WIFEXITED(child_exit_status)) {
            perror("Child process error exit");
            exit(EXIT_FAILURE);
        }

        float result;
        if (read(fd[0], &result, sizeof(result)) != sizeof(result)) {
            err(EXIT_FAILURE, "Can't read from pipe");
            exit(EXIT_FAILURE);
        }
        
        /* if (write(1, &result, sizeof(result)) != sizeof(result)) {
            err(EXIT_FAILURE, "Can't write into pipe");
            exit(EXIT_FAILURE);
        } */

        close(fd[0]);
        close(fd[1]);

        printf("\n%f\n", result);
    }

    return 0;
}
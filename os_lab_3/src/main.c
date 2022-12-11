#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int flag = 0;

typedef struct {
    int thread_number;
    int thread_quantity;
    int level;
    int chunk_size;
    int *data;
    int data_size;
} params;

#define FICTIOUS INT_MAX
#define STR_LEN 32

void *thread_function(void *param) {
    params args = * (params *) param;

    int num = args.thread_number;
    int *a = args.data;
    int size = args.data_size;
    int total = args.thread_quantity;
    int chunk_size = args.chunk_size;
    int level = args.level;
    flag = 1;

    while (num < size / 2) {
        int i = (num % chunk_size) + 2 * chunk_size * (num / chunk_size);
        int j = i + chunk_size;
        if ((i / (1 << level)) % 2 == 0) {
            if (a[i] > a[j]) {
                int temp = a[j];
                a[j] = a[i];
                a[i] = temp;
            }
        }
        if ((i / (1 << level)) % 2 == 1) {
            if (a[i] < a[j]) {
                int temp = a[j];
                a[j] = a[i];
                a[i] = temp;
            }
        }
        num += total;
    }
}

unsigned nearest_power_of_2(unsigned x) {
    int n = 1;
    int t = 0;
    while (n < x) {
        n <<= 1;
        t++;
    }
    return 1 << t;
}

size_t strlen_new(char *s) {
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] == '\n')
            return i;
    }
    return strlen(s);
}

char *get_name(const char *text) {
    printf("%s\n", text);

    char *fname_buf = calloc(sizeof(char), STR_LEN);
    if (read(0, fname_buf, STR_LEN) < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    char *fname = malloc(sizeof(char) * strlen_new(fname_buf));
    strncpy(fname, fname_buf, strlen_new(fname_buf));
    free(fname_buf);

    return fname;
}

int read_int(int fd, int *f) {
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

    *f = atoi(buf);

    if (c == '\n') {
        return 0;
    }
    
    return 1;
}

char *itoa(int x) {
    int n = 1;
    int a = abs(x);
    while (a >= 10) {
        a /= 10;
        n++;
    }
    char *result;
    if (x < 0) {
        result = calloc(sizeof(char), n + 1);
        a = abs(x);
        for (int i = n; i > 0; --i) {
            result[i] = a % 10 + 48;
            a /= 10;
        }
        result[0] = '-';
    }
    else {
        result = calloc(sizeof(char), n);
        a = x;
        for (int i = n - 1; i >= 0; --i) {
            result[i] = a % 10 + 48;
            a /= 10;
        }
    }
    return result;    
}

int main(int argc, const char **argv) {

    if (argc != 2) {
        char *err = "Error: number of threads expected\n";
        write(2, err, strlen(err));
        exit(EXIT_FAILURE);
    }

    char *array_name = get_name("Enter name of file with array to sort: ");
    char *output_name = get_name("Enter name of file for result: ");
    
    int filedes;

    if ((filedes = open(array_name, O_RDONLY)) < 0) {
        perror(array_name);
        exit(EXIT_FAILURE);
    }

    int n;
    read_int(filedes, &n);

    int m = nearest_power_of_2(n);

    int *a = calloc(sizeof(int), m);
    for (int i = 0; i < m; ++i) {
        if (i < n) {
            read_int(filedes, &a[i]);
        }
        else {
            a[i] = FICTIOUS;
        }
    }
    printf("\n");

    int thread_number = atoi(argv[1]);

    pthread_t thread_id[thread_number];

    int max_level = 0;
    int i = 1;
    while (i < m) {
        i *= 2;
        max_level++;
    }

    clock_t start, end;
    start = clock();

    for (int level = 1; level <= max_level; ++level) {
        for (int chunk_size = 1 << (level - 1); chunk_size > 0; chunk_size /= 2) {
            for (int i = 0; i < thread_number; ++i) {
                params t = {i, thread_number, level, chunk_size, a, m};
                int status = pthread_create(&thread_id[i], NULL, thread_function, &t);
                if (status != 0) {
                    perror("Thread create error");
                    exit(EXIT_FAILURE);
                }
            }
            for (int i = 0; i < thread_number; ++i) {
                pthread_join(thread_id[i], NULL);
            }
        }
    }

    end = clock(); 
    close(filedes);
    if ((filedes = open(output_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU)) < 0) {
        perror(output_name);
        exit(EXIT_FAILURE);
    }

    a = realloc(a, n * sizeof(int));

    for (int i = 0; i < n; ++i) {
        char *numb = itoa(a[i]);
        write(filedes, numb, strlen(numb));
        char space = ' ';
        write(filedes, &space, 1);
    }

    close(filedes);

    printf("%lf\n", (float)(end - start) / (CLOCKS_PER_SEC));

    
}
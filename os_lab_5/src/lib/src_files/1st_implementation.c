#include "../functions.h"

double sin_integral(double a, double b, double e) {
    if (a >= b) {
        printf("Invalid segment borders\n");
        return 0;
    }
    if (e > (b - a)) {
        printf("Step is bigger than segment size\n");
        return 0;
    }
    double i = 0;
    double result = 0;
    while (a + i <= b) {
        result += sin(a + i) * e;
        i += e;
    }
    printf("\nCalculated via rectangle method: ");
    return result;
}

int *sort(int *b, int n) {
    int *a = (int *)calloc(sizeof(int), n);
    for (int i = 0; i < n; ++i) {
        a[i] = b[i];
    }
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (a[j + 1] < a[j]) {
                int k = a[j];
                a[j] = a[j + 1];
                a[j + 1] = k;
            }
        }
    }
    printf("\nSorted by bubble sort algorithm: \n");
    return a;
}
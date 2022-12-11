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
    while (a + i < b) {
        result += (sin(a + i) + sin(a + i + e)) * e / 2;
        i += e;
    }
    printf("\nCalculated via trapezoid method: ");
    return result;
}

void quicksort(int* a, int first, int last) {
    int pivot = a[(first + last) / 2];
    int i = first, j = last;
    do {
        while(a[i] < pivot) {
            i++;
        }
        while(a[j] > pivot) {
            j--;
        }
        if (i <= j) {
            if (i < j) {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
            i++;
            j--;
        }
    } while(i <= j);
    if (i < last) {
        quicksort(a, i, last);
    }
    if (j > first) {
        quicksort(a, first, j);
    }
}

int *sort(int *b, int n) {
    int *a = calloc(sizeof(int), n);
    for (int i = 0; i < n; ++i) {
        a[i] = b[i];
    }
    quicksort(a, 0, n - 1);
    printf("\nSorted by quicksort algorithm\n");
    return a;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

double max_el(double *restrict a, double *restrict b)
{
    return (*a) > (*b) ? (*a) : (*b);
}

void insert_sort(double *restrict M, int from, int to)
{
    int key = 0;
    double temp = 0.0;
    for (int k = from; k < to; k++)
    {
        key = k + 1;
        temp = M[key];
        for (int j = k + 1; j > from; j--)
        {
            if (temp < M[j - 1])
            {
                M[j] = M[j - 1];
                key = j - 1;
            }
        }
        M[key] = temp;
    }
}

int main(int argc, char *argv[])
{
    int i, N;
    struct timeval T1, T2;
    long delta_ms;
    double Abeg = 1.0;
    double A = 315.0;
    double Aend = A * 10.0;
    unsigned int seed;
    N = atoi(argv[1]);       /* N равен первому параметру командной строки */
    gettimeofday(&T1, NULL); /* запомнить текущее время T1 */
    int N_2 = N / 2;
    double *restrict M1 = malloc(N * sizeof(double));
    double *restrict M2 = malloc(N_2 * sizeof(double));
    double *restrict M2_old = malloc(N_2 * sizeof(double));
    double X;
    unsigned int *restrict seedp = &seed;

    for (i = 0; i < 100; i++) /* 100 экспериментов */
    // for (i = 0; i < 5; i++) /* 5 экспериментов */
    {
        /* инициализировать начальное значение ГСЧ */
        seed = i;

        /* Заполнить массив исходных данных размером N */
        // GENERATE
        for (int j = 0; j < N; j++)
        {
            M1[j] = ((double)rand_r(seedp) / (RAND_MAX)) * (A - Abeg) + Abeg;
        }
        /* Заполнить массив исходных данных размером N/2 */
        for (int k = 0; k < N_2; k++)
        {
            M2[k] = ((double)rand_r(seedp) / (RAND_MAX)) * (Aend - A) + A;
        }

        // MAP
        for (int j = 0; j < N; j++)
        {
            M1[j] = cbrt(M1[j] / exp(1));
        }
        for (int k = 0; k < N_2; k++)
        {
            M2_old[k] = M2[k];
        }
        M2[0] = fabs(cos(M2[0]));
        for (int k = 1; k < N / 2; k++)
        {
            M2[k] = M2[k] + M2_old[k - 1];
            M2[k] = fabs(cos(M2[k]));
        }

        // MERGE
        for (int k = 0; k < N / 2; k++)
        {
            M2[k] = max_el(&M1[k], &M2[k]);
        }

        /* Отсортировать массив с результатами указанным методом */
        // SORT
        // insert_sort(M2, 0, N_2);

        // REDUCE
        X = 0.0;

        // поиск минимального ненулевого элемента массива M2
        int k = 0;
        while (M2[k] == 0 && k < N_2 - 1)
            k++;
        double minelem = M2[k];
        
        // sum of matching array elements
        for (int k = 0; k < N_2; k++)
        {
            M2_old[k] = 0.0;
            if ((int)(M2[k] / minelem) % 2 == 0)
            {
                M2_old[k] = sin(M2[k]);
            }
        }

        for (int j = 0; j < N_2; ++j)
        {
            X += M2_old[j];
        }
        //printf("X = %f ", X);
        //printf("\n\n");
    }
    gettimeofday(&T2, NULL); /* запомнить текущее время T2 */
    delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 - T1 */
    // printf("%ld\n", delta_ms);
    return 0;
}

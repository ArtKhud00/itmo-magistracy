#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "Framewave/fwBase.h"
#include "Framewave/fwSignal.h"


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
    N = atoi(argv[1]);                           /* N равен первому параметру командной строки */
    unsigned int Threads_Number = atoi(argv[2]); /* Количество ядер - второй параметр командной строки */
    fwSetNumThreads(Threads_Number);
    gettimeofday(&T1, NULL); /* запомнить текущее время T1 */
    int N2 = N / 2;          /* N2 равен N/2*/
    Fw32f *restrict M1 = (Fw32f *)fwMalloc(N * sizeof(Fw32f));
    Fw32f *restrict M2 = (Fw32f *)fwMalloc(N2 * sizeof(Fw32f));
    Fw32f *restrict M2_old = (Fw32f *)fwMalloc(N2 * sizeof(Fw32f));
    Fw32f *restrict expon = (Fw32f *)fwMalloc(sizeof(Fw32f));
    *expon = (Fw32f)exp(1); // константа для экспоненты
    double Abeg = 1.0;
    double A = 315.0;
    double Aend = A * 10.0;
    double X;
    //int j, k;
    unsigned int seed;
    unsigned int *restrict seedp = &seed;
    int len1 = N;
    int len2 = N2;
    for (i = 0; i < 100; ++i) /* 100 экспериментов */
    // for (i=0; i < 5; ++i) /* 5 экспериментов */
    { 
        
        /* инициализировать начальное значение ГСЧ */
        seed = i;
        /* Заполнить массив исходных данных размером N */
        // GENERATE
        for (int j = 0; j < N; ++j)
        {
            M1[j] = ((double)rand_r(seedp) / (RAND_MAX)) * (A - Abeg) + Abeg;
        }
        for (int k = 0; k < N2; ++k)
        {
            M2[k] = ((double)rand_r(seedp) / (RAND_MAX)) * (Aend - A) + A;
        }

        // MAP
        // Делим все элементы на экспоненту
        fwsDivC_32f_I(*expon, M1, len1);
        // Находмим кубический корень каждого из элементов массива
        fwsCubrt_32f(M1, M1, len1);
        // Создаем копию массива M2
        fwsCopy_32f(M2, M2_old, len2); // OK

        fwsAdd_32f_I(M2_old, M2 + 1, len2 - 1);
        fwsCos_32f_A24(M2, M2, len2);
        fwsAbs_32f_I(M2, len2);

        // MERGE
        fwsMaxEvery_32f_I(M1, M2, len2);

        // SORT
        // insert_sort(M2, 0, N_2);

        // REDUCE
        X = 0.0;
        int k = 0;
        while (M2[k] == 0 && k < N2 - 1)
            k++;
        double minelem = M2[k];

        // sum of matching array elements        
        for (int k = 0; k < N2; k++)
        {
            M2_old[k] = 0.0;
            if ((int)(M2[k] / minelem) % 2 == 0)
            {
                M2_old[k] = sin(M2[k]);
            }
        }

        for (int j = 0; j < N2; ++j)
        {
            X += M2_old[j];
        }
        //  printf("X = %f", X);
        //   printf("\n\n");
    }
    gettimeofday(&T2, NULL); /* запомнить текущее время T2 */
    delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 - T1 */
    // printf("%d %ld\n", N, delta_ms);
    return 0;
}

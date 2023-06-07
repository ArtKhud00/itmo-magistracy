#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

// #define SCHEDULE dynamic
// #define CHUNK 10


double max_el(double* restrict a, double* restrict b)
{
    return (*a) > (*b) ? (*a) : (*b);
}

void insert_sort(double* M, int n)
{
    int key = 0;
    double temp = 0.0;
    for (int k = 0; k < n - 1; k++)
    {
        key = k + 1;
        temp = M[key];
        for (int j = k + 1; j > 0; j--)
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

int main(int argc, char* argv[])
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
    double* restrict M1 = malloc(N * sizeof(double));
    double* restrict M2 = malloc(N_2 * sizeof(double));
    double* restrict M2_old = malloc(N_2 * sizeof(double));

    const int num_threads = atoi(argv[2]); /* amount of threads */
    #if defined(_OPENMP)
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    #endif

    double expon = exp(1);
    unsigned int seed1[num_threads];
    unsigned int seed2[num_threads];

    for (i = 0; i < 100; i++) /* 100 экспериментов */
    {
        double X = 0.0;
        seed = i;
    #pragma omp parallel default(none) shared(N, N_2, M1, M2, M2_old, X, expon, seed1, seed2, A, Abeg, Aend)
        {


        #if defined(_OPENMP)
            int tid = omp_get_thread_num();
            seed1[tid] = rand();
            seed2[tid] = rand();

            // #pragma omp for schedule(SCHEDULE, CHUNK)
        #pragma omp for
            for (int j = 0; j < N; j++) {
                int tid = omp_get_thread_num();
                unsigned int local_seed = seed1[tid] + j;
                M1[j] = ((double)rand_r(&local_seed) / (RAND_MAX)) * (A - Abeg) + Abeg;
            }
            // #pragma omp for schedule(SCHEDULE, CHUNK)
        #pragma omp for
            for (int k = 0; k < N_2; ++k) {
                int tid = omp_get_thread_num();
                unsigned int local_seed1 = seed2[tid] + k;
                M2[k] = ((double)rand_r(&local_seed1) / (RAND_MAX)) * (Aend - A) + A;
            }
        #else
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
        #endif
 
            // MAP
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif
            for (int j = 0; j < N; j++)
            {
                M1[j] = cbrt(M1[j] / expon);
            }

        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif 
            for (int k = 0; k < N_2; k++)
            {
                M2_old[k] = M2[k];
            }
        #pragma omp single
            M2[0] = fabs(cos(M2[0]));
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif
            for (int k = 1; k < N_2; k++)
            {
                M2[k] = M2[k] + M2_old[k - 1];
            }
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif
            for (int k = 1; k < N_2; ++k) {
                M2[k] = fabs(cos(M2[k]));
            }

            //  MERGE
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif
            for (int k = 0; k < N_2; k++)
            {
                M2[k] = max_el(&M1[k], &M2[k]);
            }

            /* Отсортировать массив с результатами указанным методом */
            // SORT
            // insert_sort(&M2, N_2);

            // REDUCE

            int k = 0;
            while (M2[k] == 0 && k < N_2 - 1) k++;
            double minelem = M2[k];
            
            //  sum of matching array elements
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for
        #endif
            for (int k = 0; k < N_2; k++)
            {
                M2_old[k] = 0.0;
                if ((int)(M2[k] / minelem) % 2 == 0)
                {
                    M2_old[k] = sin(M2[k]);
                }
            }
        #if defined(CHUNK) && defined(SCHEDULE)
        #pragma omp for reduction(+ : X) schedule(SCHEDULE, CHUNK)
        #else
        #pragma omp for reduction(+ : X)
        #endif
            for (int j = 0; j < N_2; ++j)
            {
                X += M2_old[j];
            }
        }
        // printf("X = %f ", X);
        // printf("%f", X);
        // printf("\n\n");
    }
    gettimeofday(&T2, NULL); /* запомнить текущее время T2 */
    delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 - T1 */
    return 0;
}

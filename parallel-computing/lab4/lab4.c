#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>


#ifdef _OPENMP
#include <omp.h>
#else
int omp_get_max_threads()
{
    return 1;
}

int omp_get_num_procs()
{
    return 1;
}

int omp_get_thread_num()
{
    return 0;
}

void omp_set_num_threads(int thrds)
{
    return;
}

double omp_get_wtime()
{
    struct timeval T;
    double time_ms;

    gettimeofday(&T, NULL);
    time_ms = (1000.0 * ((double)T.tv_sec) + ((double)T.tv_usec) / 1000.0);
    return (double)(time_ms / 1000.0);
}

void omp_set_nested(int b)
{
    return;
}
#endif

void generate_array(double* restrict m, int size, unsigned int min, unsigned int max, int seed)
{
#pragma omp for 
    for (int i = 0; i < size; ++i)
    {
        unsigned int tmp_seed = sqrt(i + seed);
        m[i] = ((double)rand_r(&tmp_seed) / (RAND_MAX)) * (max - min) + min;
    }
}

double max_el(double* restrict a, double* restrict b)
{
    return (*a) > (*b) ? (*a) : (*b);
}

int min_el(int* restrict a, int* restrict b)
{
    return (*a) < (*b) ? (*a) : (*b);
}

void copy_array(double* dst, double* src, int n) {
    for (int i = 0; i < n; ++i)
    {
        dst[i] = src[i];
    }
}

void insert_sort(double* restrict M, int from, int to, int size)
{
    int key = 0;
    double temp = 0.0;
    for (int k = from; k < to - 1; k++)
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

void merge_sorted(double* src1, int n1, double* src2, int n2, double* dst) {

    int i = 0, i1 = 0, i2 = 0;
    while (i < n1 + n2) {
        dst[i++] = src1[i1] > src2[i2] && i2 < n2 ? src2[i2++] : src1[i1++];
    }
}



void merge_sort(double* restrict MM, int size, double* restrict dst) {
    int n_threads = omp_get_num_procs();
    int chunk_size = size / n_threads;
 
    int tid = omp_get_thread_num();
    //printf("Thread num %d\n",tid);
    int start = tid * chunk_size;
    int end = (tid == n_threads - 1) ? size + 1 : (tid + 1) * chunk_size;
    insert_sort(MM, start, end, size);
    #pragma omp single
    {
        double* restrict cpy = malloc(size * sizeof(double));

        copy_array(cpy, MM, size);
        copy_array(dst, MM, size);
        for (int k = 1; k < n_threads; ++k)
        {
            int n_done = chunk_size * k;
            int vsp = size - n_done;
            int n_cur_chunk = min_el(&(vsp), &(chunk_size));
            if (k == n_threads - 1)
            {
                n_cur_chunk = size - n_done;
            }
            int n_will_done = n_done + n_cur_chunk;
            merge_sorted(cpy, n_done, MM + n_done, n_cur_chunk, dst);
            copy_array(cpy, dst, n_will_done);
        }
    }
}

void merge_sort_halves(double* restrict MM, int size, double* restrict dst, int num_threads)
{
    int n1 = size / 2;
    omp_set_num_threads(2);
#pragma omp sections
    {
#pragma omp section
        insert_sort(MM, 0, n1, size);
#pragma omp section
        insert_sort(MM, n1, size + 1, size);
    }
#pragma omp single
    merge_sorted(MM, n1, MM + n1, size - n1, dst);
    omp_set_num_threads(num_threads);
}

int mainpart(int argc, char* argv[], int* progress, int* i)
{
    int N;
    double T1, T2;
    long long delta_ms;
    double Abeg = 1.0;
    double A = 315.0;
    double Aend = A * 10.0;
    unsigned int seed;
    N = atoi(argv[1]);       /* N равен первому параметру командной строки */
    //gettimeofday(&T1, NULL); /* запомнить текущее время T1 */
    T1 = omp_get_wtime();
    int N_2 = N / 2;
    double* restrict M1 = malloc(N * sizeof(double));
    double* restrict M2 = malloc(N_2 * sizeof(double));
    double* restrict M2_old = malloc(N_2 * sizeof(double));
    double* restrict M2_sorted = malloc(N_2 * sizeof(double));
    const int num_threads = atoi(argv[2]); /* amount of threads */
#if defined(_OPENMP)
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
#endif

    double expon = exp(1);

    for (int j = 0; j < 100; j++) /* 100 экспериментов */
    {
        /* инициализировать начальное значение ГСЧ */
        double X = 0.0;
        seed = j;
        *i = j;




    #pragma omp parallel default(none) shared(N, N_2, M1, M2, M2_old, M2_sorted, X, expon,  A, Abeg, Aend, num_threads, seed) 
        {
            /* Заполнить массив исходных данных размером N */
            // GENERATE
            generate_array(M1, N, Abeg, A, seed);
            generate_array(M2, N_2, A, Aend, seed + 2);
            // MAP
        #pragma omp for 
            for (int j = 0; j < N; j++)
            {
                M1[j] = cbrt(M1[j] / expon);
            }

        #pragma omp for nowait 
            for (int k = 0; k < N_2; k++)
            {
                M2_old[k] = M2[k];
            }
        #pragma omp single
            M2[0] = fabs(cos(M2[0]));

        #pragma omp for 
            for (int k = 1; k < N_2; k++)
            {
                M2[k] = M2[k] + M2_old[k - 1];
            }

        #pragma omp for 
            for (int k = 1; k < N_2; ++k)
            {
                M2[k] = fabs(cos(M2[k]));
            }

            //  MERGE
        #pragma omp for 
            for (int k = 0; k < N_2; k++)
            {
                M2[k] = max_el(&M1[k], &M2[k]);
            }

            /* Отсортировать массив с результатами указанным методом */
            // SORT
             if(num_threads == 2)
             {
                 merge_sort_halves(M2,N_2, M2_sorted);
             }
             else
             {
                 merge_sort(M2, N_2, M2_sorted);
             }

            // REDUCE
            int k = 0;
            while (M2_sorted[k] == 0 && k < N_2 - 1)
                k++;
            double minelem = M2_sorted[k];
            //  sum of matching array elements
            #pragma omp for              
            for (int k = 0; k < N_2; k++)
            {
                M2_old[k] = 0.0;
                if ((int)(M2_sorted[k] / minelem) % 2 == 0)
                {
                    M2_old[k] = sin(M2_sorted[k]);
                }
            }

            #pragma omp for reduction(+: X) 
            for (int j = 0; j < N_2; ++j)
            {
                X += M2_old[j];
            }
        }
        // printf("X = %f\n ", X);;
    }
    *progress = 1;
    T2 = omp_get_wtime();
    delta_ms = 1000 * (T2 - T1);
    // printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 - T1 */
    printf("%lld\n", delta_ms);
    // printf("%lld\n", delta_ms);
    return 0;
}

void progressnotifier(int* progress, int* i)
{
    double time = 0;
    while (*progress < 1)
    {
        double time_temp = omp_get_wtime();
        if (time_temp - time < 1)
        {
            usleep(100);
            continue;
        };
        //printf("\nPROGRESS: %d\n", *i);
        time = time_temp;
    }
}

int main(int argc, char* argv[])
{
    double T1, T2;
    int* progress = malloc(sizeof(int));
    *progress = 0;
    int* i = malloc(sizeof(int));
    *i = 0;
    omp_set_nested(1);
    #pragma omp parallel sections num_threads(2) shared(i, progress)
    {
        #pragma omp section
            progressnotifier(progress, i);
        #pragma omp section
            mainpart(argc, argv, progress, i);
    }
    return 0;
}
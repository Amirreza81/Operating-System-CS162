#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>


#define SIZE 5000

int A[SIZE][SIZE];
int B[SIZE][SIZE];
int C[SIZE][SIZE];

int thread_pool_size;
int M;
int K;
int N;

int temp;

void *multiply(void *arg) {
    int thread_id = *(int *) (arg);
    int start = thread_id * M;
    int end = (thread_id + 1) * M;
    for (int i = start / thread_pool_size;
         i < end / thread_pool_size; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    struct timeval startTime, endTime;
    scanf("%d %d %d", &M, &K, &N);

    int sizes[3] = {1, 2, 4};

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            A[i][j] = i + 1;
        }
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < N; j++) {
            B[i][j] = j + 1;
        }
    }

    for (int i = 0; i < 3; i++) {
        thread_pool_size = sizes[i];
        pthread_t threads[thread_pool_size];
        gettimeofday(&startTime, NULL);

        int a[thread_pool_size];

        for (int j = 0; j < thread_pool_size; j++) {
            a[j] = j;
            pthread_create(&threads[j], NULL, multiply, &a[j]);
        }

        // wait for threads

        for (int j = 0; j < thread_pool_size; j++) {
            pthread_join(threads[j], NULL);
        }

        gettimeofday(&endTime, NULL);
        printf("Thread Pool Size: \t%d\n", thread_pool_size);
        float end_time = (endTime.tv_sec - startTime.tv_sec) * 1000.0;
        float start_time = (endTime.tv_usec - startTime.tv_usec) / 1000.0;
        float final_time = end_time + start_time;
        printf("Time of execution: \t%.4lf ms\n", final_time);
        printf("---------------------------------------------------\n");
    }
}

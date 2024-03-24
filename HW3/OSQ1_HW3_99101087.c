#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000

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
    for (int i = thread_id * M / thread_pool_size;
         i < (thread_id + 1) * M / thread_pool_size; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}


int main() {
    scanf("%d", &thread_pool_size);

    scanf("%d %d", &M, &K);
    // A -> M*K
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            scanf("%d", &temp);
            A[i][j] = temp;
            temp = 0;
        }
    }

    scanf("%d %d", &K, &N);
    // B  -> K*N
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < N; j++) {
            scanf("%d", &temp);
            B[i][j] = temp;
            temp = 0;
        }
    }

    // number of threads
    pthread_t threads[thread_pool_size];

    int a[thread_pool_size];

    for (int i = 0; i < thread_pool_size; i++) {
        a[i] = i;
        pthread_create(&threads[i], NULL, multiply, &a[i]);
    }

    // wait for threads

    for (int i = 0; i < thread_pool_size; i++) {
        pthread_join(threads[i], NULL);
    }

    // C -> M*N
    printf("Result: \n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
    printf("Size of the final matrix is: %d * %d\n", M, N);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#define PI 3.14159265358979323846

typedef struct {
    int term;  // Номер члена ряда
    double value;  // Значение члена ряда
    int i;  // Индекс i в y[i]
    int N;  // Значение N
} TaylorTerm;

double factorial(int n) {
    if (n == 0 || n == 1) return 1;
    double result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Функция, вычисляющая один член ряда Тейлора
void *calculate_term(void *arg) {
    TaylorTerm *termData = (TaylorTerm *)arg;
    int k = termData->term;
    int i = termData->i;
    int N = termData->N;

    termData->value = pow(-1, k) * pow(2 * PI * i / N, 2 * k + 1) / factorial(2 * k + 1);

    printf("Поток PID=%d, член ряда %d, значение: %.10f\n", getpid(), k, termData->value);

    pthread_exit(NULL);
}

int main() {
    int K, N, n;
    
    printf("Введите K (количество значений y[i]): ");
    scanf("%d", &K);
    printf("Введите N: ");
    scanf("%d", &N);
    printf("Введите n (количество членов ряда Тейлора): ");
    scanf("%d", &n);

    FILE *file = fopen("results.txt", "w");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    for (int i = 0; i < K; i++) {
        pthread_t threads[n]; 
        TaylorTerm terms[n];
        double y_i = 0;

        // Создание потоков для каждого члена ряда
        for (int k = 0; k < n; k++) {
            terms[k].term = k;
            terms[k].i = i;
            terms[k].N = N;

            pthread_create(&threads[k], NULL, calculate_term, (void *)&terms[k]);
        }

        // Ожидание завершения всех потоков и суммирование результатов
        for (int k = 0; k < n; k++) {
            pthread_join(threads[k], NULL);
            y_i += terms[k].value;
        }

        printf("y[%d] = %.10f\n", i, y_i);
        fprintf(file, "y[%d] = %.10f\n", i, y_i);
    }

    fclose(file);
    printf("Результаты записаны в results.txt\n");

    return 0;
}

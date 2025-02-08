#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#define PI 3.14159265358979323846

typedef struct {
    int term;   // Номер члена ряда
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
double calculate_term(int k, int i, int N) {
    return pow(-1, k) * pow(2 * PI * i / N, 2 * k + 1) / factorial(2 * k + 1);
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
        double y_i = 0;
        int pipefd[2];

        if (pipe(pipefd) == -1) {
            perror("Ошибка создания канала");
            return 1;
        }

        for (int k = 0; k < n; k++) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("Ошибка при fork()");
                return 1;
            } else if (pid == 0) { // Дочерний процесс
                double term_value = calculate_term(k, i, N);
                printf("Процесс PID=%d, член ряда %d, значение: %.10f\n", getpid(), k, term_value);
                
                close(pipefd[0]); // Закрываем чтение
                write(pipefd[1], &term_value, sizeof(double));
                close(pipefd[1]); // Закрываем запись
                
                exit(0);
            }
        }

        // Родительский процесс ждет всех потомков и собирает результаты
        close(pipefd[1]); // Закрываем запись
        for (int k = 0; k < n; k++) {
            double term_value;
            read(pipefd[0], &term_value, sizeof(double));
            y_i += term_value;
            wait(NULL); // Ожидание завершения всех дочерних процессов
        }
        close(pipefd[0]); // Закрываем чтение

        printf("y[%d] = %.10f\n", i, y_i);
        fprintf(file, "y[%d] = %.10f\n", i, y_i);
    }

    fclose(file);
    printf("Результаты записаны в results.txt\n");

    return 0;
}

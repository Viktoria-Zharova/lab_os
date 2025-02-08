#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#define PI 3.14159265358979323846

// Функция для вычисления факториала
double factorial(int n) {
    if (n == 0 || n == 1) return 1.0;
    double result = 1.0;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Функция для вычисления члена ряда Тейлора для sin(x)
double taylor_series_term(int k, double x) {
    return pow(-1, k) * pow(x, 2 * k + 1) / factorial(2 * k + 1);
}

int main() {
    int K, N, n;

    // Ввод данных
    printf("Введите K (количество значений y[i]): ");
    scanf("%d", &K);
    printf("Введите N: ");
    scanf("%d", &N);
    printf("Введите n (количество членов ряда Тейлора): ");
    scanf("%d", &n);

    // Открытие файла для записи результатов
    FILE *file = fopen("results.txt", "w");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    // Переменные для вычисления
    double y_i;
    int pipefd[2];

    // Для каждого i в пределах от 0 до K-1
    for (int i = 0; i < K; i++) {
        y_i = 0;  // Обнуляем значение для каждого i
        if (pipe(pipefd) == -1) {
            perror("Ошибка при создании канала");
            return 1;
        }

        // Для каждого члена ряда
        for (int k = 0; k < n; k++) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("Ошибка при fork()");
                return 1;
            } else if (pid == 0) {  // Дочерний процесс
                double x = (2 * PI * i) / N;  // Вычисление x = 2 * PI * i / N
                double term_value = taylor_series_term(k, x);  // Вычисление члена ряда

                // Вывод PID и значения члена ряда
                printf("Процесс PID=%d, член ряда %d, значение: %.10f\n", getpid(), k, term_value);

                // Запись результата в pipe
                close(pipefd[0]);  // Закрыть чтение
                write(pipefd[1], &term_value, sizeof(double));  // Записать результат
                close(pipefd[1]);  // Закрыть запись

                exit(0);  // Завершаем дочерний процесс
            }
        }

        // Родительский процесс ожидает завершения всех дочерних
        close(pipefd[1]);  // Закрыть запись
        for (int k = 0; k < n; k++) {
            double term_value;
            read(pipefd[0], &term_value, sizeof(double));  // Чтение результата из pipe
            y_i += term_value;  // Суммирование члена ряда
            wait(NULL);  // Ожидание завершения дочернего процесса
        }
        close(pipefd[0]);  // Закрыть чтение

        // Вывод результата для y[i]
        printf("y[%d] = %.10f\n", i, y_i);
        fprintf(file, "y[%d] = %.10f\n", i, y_i);  // Запись в файл
    }

    fclose(file);  // Закрытие файла
    printf("Результаты записаны в results.txt\n");

    return 0;
}

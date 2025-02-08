#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void print_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info = localtime(&ts.tv_sec);
    
    printf("Текущее время: %02d:%02d:%02d:%03ld\n",
           tm_info->tm_hour,
           tm_info->tm_min,
           tm_info->tm_sec,
           ts.tv_nsec / 1000000);
}

int main() {
    pid_t pid1, pid2;

    printf("Родительский процесс PID=%d\n", getpid());
    print_time();

    pid1 = fork(); // Первый вызов fork()
    
    if (pid1 == 0) {
        // Код дочернего процесса 1
        printf("Дочерний процесс 1 PID=%d, родительский PID=%d\n", getpid(), getppid());
        print_time();
        exit(0);
    } 
    else if (pid1 > 0) {
        // Родительский процесс продолжает выполнение
        pid2 = fork(); // Второй вызов fork()

        if (pid2 == 0) {
            // Код дочернего процесса 2
            printf("Дочерний процесс 2 PID=%d, родительский PID=%d\n", getpid(), getppid());
            print_time();
            exit(0);
        }
        else if (pid2 > 0) {
            // Родительский процесс ждет завершения потомков
            wait(NULL);
            wait(NULL);

            printf("Родительский процесс выполняет команду ps -x с использованием grep lab_fork.c:\n");
            // Используем команду ps -x, чтобы найти процессы, связанные с lab_fork.c
            system("ps -x | grep lab_fork.c"); // Выполняем команду, чтобы найти процессы по имени программы
        }
        else {
            printf("Ошибка создания второго дочернего процесса\n");
        }
    }
    else {
        printf("Ошибка создания первого дочернего процесса\n");
    }

    return 0;
}
